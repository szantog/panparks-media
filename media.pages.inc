<?php
// $Id$

/**
 * @file
 * Common pages for the Media module.
 */

/**
 * Menu callback; view a single piece of media.
 */
function media_file_view($media) {
  drupal_set_title($media->filename);
  // @TODO maybe something more elegant... ?
  field_attach_prepare_view('media', array($media->fid => $media), 'media_original');
  $build['media'][$media->fid] = field_attach_view('media', $media, 'media_original');

  return $build;
}

/**
 *  Menu callback; display a thumbnail for an AJAX preview.
 */
function media_preview_ajax() {
  $url = $_GET['url'];
  $uri = media_parse_to_uri($url);
  if ($uri) {
    $media = file_uri_to_object($uri);
    drupal_json_output(array(
      'preview' => theme('media_admin_thumbnail', array('file' => $media)),
    ));
  }
  else {
    drupal_json_output(array(
      'error' => t('Invalid URL.'),
    ));
  }
  die();
}

/**
 * Menu callback; presents the Media editing form.
 */
function media_page_edit($media) {
  drupal_set_title(t('Edit @title', array('@title' => drupal_set_title($media->filename))));
  return drupal_get_form('media_edit', $media);
}

/**
 * Menu callback; presents the Media editing form for multiple items.
 */
function media_page_multiedit($media_items) {
  if (!module_exists('multiform')) {
    drupal_set_message('To edit multiple media items, you must install the multiform module.');
  }
  $i = 0;
  foreach ($media_items as $item) {
    // To maintain unique form_ids, increment this counter.
    // @see media_forms().
    $i++;
    $forms[] = array("media_edit_$i", $item);
  }
  
  $form = call_user_func_array('multiform_get_form', $forms);
  $form['#attributes']['class'][] = 'media-multiedit-form';
  
  unset($form['buttons']['Delete']);
  // Would be nice to add this to show a message, but not working.
  // Can debug.
  //$form['buttons']['Save']['#submit'][] = 'media_page_multiedit_submit';
  
  drupal_set_title(t('Editing multiple media files'));
  return $form;
}

/**
 * Menu callback; shows delete confirmation form.
 */
function media_page_delete($media) {
  $files = array($media->fid => $media);
  return drupal_get_form('media_multiple_delete_confirm', $files);
}


/**
 * Confirm the request to delete files.
 */
function media_multiple_delete_confirm($form, &$form_state, $files) {
  $form['files'] = array('#prefix' => '<ul>', '#suffix' => '</ul>', '#tree' => TRUE);
  // array_filter returns only elements with TRUE values
  foreach ($files as $fid => $value) {
    $title = db_query('SELECT filename FROM {file_managed} WHERE fid = :fid', array(':fid' => $fid))->fetchField();
    $form['files'][$fid] = array(
      '#type' => 'hidden',
      '#value' => $fid,
      '#prefix' => '<li>',
      '#suffix' => check_plain($title) . "</li>\n",
    );
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');
  $form['#submit'][] = 'media_multiple_delete_confirm_submit';

  $confirm_question = format_plural(count($files),
                                  'Are you sure you want to delete this item?',
                                  'Are you sure you want to delete these items?');

  return confirm_form($form,
                    $confirm_question,
                    $_GET['q'], t('This action cannot be undone.'),
                    t('Delete'), t('Cancel'));
}

/**
 * Attempt to delete files and notify the user of the result.
 */
function media_multiple_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $results = array();
    $files = array_keys($form_state['values']['files']);
    foreach ($files as $fid) {
      $file = file_load($fid);
      $files[$fid] = $file;
      $results[$fid] = file_delete($file);
    }
    // The result of file_delete can be an array if the file is in use, or TRUE/FALSE.
    foreach ($results as $fid => $result) {
      if (is_array($result)) {
        drupal_set_message(t('The file @title is in use and cannot be deleted.', array('@title' => $files[$fid]->filename)), 'warning');
      }
      elseif (!$result) {
        drupal_set_message(t('The file @title was not deleted due to an error.', array('@title' => $files[$fid]->filename)), 'error');
      }
      else {
        $message = t('File @title was deleted', array('@title' => $files[$fid]->filename));
        watchdog('media', $message);
        drupal_set_message($message);
      }
    }
  }
}


/**
 * Form callback for adding media via an upload form.
 * @todo: should use the AJAX uploader
 */
function media_add_upload($form, &$form_state, $types = NULL) {

  $validators = array(
   'file_validate_extensions' => array(media_variable_get('file_extensions')),
   'file_validate_size' => array(parse_size(media_variable_get('max_filesize'))),
  );

  //@todo add additional validators for media types allowed.
  if ($types) {
    $validators['media_file_validate_types'] = array($types);
  }
  
  // A blank set of allowed file extensions means no need to validate.
  if (!$validators['file_validate_extensions'][0]) {
    unset($validators['file_validate_extensions']);
  }
  // Use the PHP limit for filesize if no variable was set.
  if (!$validators['file_validate_size']) {
    $validators['file_validate_size'] = file_upload_max_size();
  }

  if ($validators['file_validate_size'][0] == 0) {
    unset($validators['file_validate_size']);
  }

  $form['#validators'] = $validators;

  $form['upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload a new file'),
    '#description' => theme('file_upload_help', array('description' => '', 'upload_validators' => $validators)),
    '#upload_validators' => $validators,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Validate the generic file upload with the global media settings.
 */
function media_add_upload_validate($form, &$form_state) {
  // Save the file as a temporary file.
  $file = file_save_upload('upload', $form['#validators']);

  if ($file === NULL) {
    form_set_error('upload', t("No file appears to have been selected."));
  }
  elseif ($file === FALSE) {
    form_set_error('upload', t("Failed to upload the file."));
  }
  else {
    $form_state['values']['upload'] = $file;
  }
}

/**
 * Upload a file.
 */
function media_add_upload_submit($form, &$form_state) {
  $scheme = variable_get('file_default_scheme', 'public') . '://';
  $file = $form_state['values']['upload'];
  _media_save_file_permenently($file);

  $destination = file_stream_wrapper_uri_normalize($scheme . $file->filename);

  $defaults = array (
    'display' => TRUE,
  );

  // Save the uploaded file.
  $file = file_move($file, $destination, FILE_EXISTS_RENAME);

  if ($file) {
    drupal_set_message(t('The file @name was uploaded', array('@name' => $file->filename)));
  }
  else {
    drupal_set_message(t('An error occurred and no file was uploaded.'), 'error');
    return;
  }

  $form_state['redirect'] = array('media/browser', array('query' => array('render' => 'media-popup', 'fid' => $file->fid)));
}

/**
 *  Callback for /media/add/upload and /admin/content/media/add/upload.
 *  @TODO: work on the description.
 *  @TODO: allow stream wrappers to add to the description.
 *  @see media_media_browser_plugins().
 */
function media_add_from_url($form, &$form_state = array(), $types = NULL) {
  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => 'URL',
    '#description' => 'Input any url which Drupal can handle',
    '#attributes' => array('class' => array('media-add-from-url')),
  );
  
  $form['preview'] = array(
    '#type' => 'item',
    '#title' => t('Preview'),
    '#markup' => '<div id="media-add-from-url-preview"></div>'
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  $form['#validators'] = array();
  if ($types) {
    $form['#validators']['media_file_validate_types'] = array($types);
  }

  // Add the javascript for live previews on textfield change.
  $form['#attached'] = array(
    'js' => array(drupal_get_path('module', 'media') . '/javascript/media-add_from_url.js'),
  );

  $settings = array(
    'media' => array(
      'add_from_url_preview' => url('media/js'),
    ),
  );
  drupal_add_js($settings, array('type' => 'setting'));

  return $form;
}

/**
 *  Allow stream wrappers to have their chance at validation.
 *
 *  Any module that implements hook_media_parse will have an
 *  opportunity to validate this.
 *
 *  @see media_parse_to_uri().
 */
function media_add_from_url_validate($form, &$form_state) {
  // Supported Schemes for media:
  $supported_schemes = media_variable_get('fromurl_supported_schemes');
  $scheme = file_uri_scheme($form_state['values']['url']);
  if (!in_array($scheme, $supported_schemes)) {
    form_set_error('url', t('Invalid URL: Only the following types of urls are supported: @supported_schemes', array('@supported_schemes' => implode(', ', $supported_schemes))));
    return;
  }
  
  // We have a dry-run of the parsing process to allow specific streams to
  // catch an error, for instance to inform us that a video is no longer
  // available or that a specific URL structure is not supported.
  try {
    // Returns the URI, or NULL on failure.
    $uri = media_parse_to_uri($form_state['values']['url']);
  }
  catch (Exception $e) {
    // Pass the error back to the user.
    form_set_error('url', $e->getMessage());
    return;
  }

  // This is just stupid, but I need to validate something here.
  // @TODO: We really should retreive the file and validate its mime-type here.
  // This is not a reliable way to get it.

  if (!$uri) {
    $uri = $form_state['values']['url'];
  }
  $validators = $form['#validators'];
  if ($validators) {
    $file = file_uri_to_object($uri);
    // Check for errors. @see media_add_upload_validate calls file_save_upload().
    // this code is ripped from file_save_upload because we just want the validation part.
    // Call the validation functions specified by this function's caller.
    $errors = file_validate($file, $validators);
    
    if (!empty($errors)) {
      $message = t('%url could not be added.', array('%url' => $form_state['values']['url']));
      if (count($errors) > 1) {
        $message .= theme('item_list', array('items' => $errors));
      }
      else {
        $message .= ' ' . array_pop($errors);
      }
      form_set_error('url', $message);
      return FALSE;
    }
  }

  if (empty($uri)) {
    // @todo - at the moment we don't actual do any real validation.
    // form_set_error('url', t('Invalid URL'));
  }
  // @TODO: Validate that if we have no $uri that this is a valid file to
  // save. For instance, we may only be interested in images, and it would
  // be helpful to let the user know they passed the HTML page containing
  // the image accidentally. That would also save us from saving the file
  // in the submit step.
  
  // This is kinda a hack of the same.

  // This should use the file_validate routines that the upload form users.
  // We need to fix the media_parse_to_file routine to allow for a validation.
}

/**
 *  Upload a file from a URL.
 *
 *  This will copy a file from a remote location and store it locally.
 *  @see media_parse_to_uri().
 *  @see media_parse_to_file().
 */
function media_add_from_url_submit($form, &$form_state) {
  $url = $form_state['values']['url'];
  $uri = ''; // The URI we are eventually saving to the files table

  // Save the remote file
  // @TODO: We should really separate parsing the URL, retreiving the file
  // and inserting into different operations.
  try {
    $file = media_parse_to_file($url);
  }
  catch (Exception $e) {
    form_set_error('url', $e->getMessage());
    return;
  }

  if (!$file->fid) {
    form_set_error('url', 'Unknown error: unable to add file, please check URL and try again ' . $url);
    return;
  }

  $form_state['redirect'] = array('media/browser', array('query' => array('render' => 'media-popup', 'fid' => $file->fid)));
}

/**
 * Form builder: Builds the edit file form.
 */
function media_edit($form, $form_state, $media) {
  drupal_set_title($media->filename);
  field_attach_form('media', $media, $form, $form_state);
  unset($form['file']);

  $form['#attached'] = array(
    'css' => array(drupal_get_path('module', 'media') . '/css/media.css'),
  );

  // Not sure about this class name, seems to indicate the style.
  $form['#attributes']['class'][] = 'media-image-left';
  $form['#attributes']['class'][] = 'media-edit-form';
  
  $form['preview'] = field_view_field('media', $media, 'file', 'media_preview');
  $form['preview']['#weight'] = -10;
  $form['preview']['#suffix'] = '<div class="no-overflow">';

  // Add the buttons.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['#prefix'] = '</div>';
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#weight' => 15,
    '#submit' => array('media_edit_delete_submit'),
  );
  
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('media_edit_submit'),
  );

  $form['fid'] = array(
    '#type' => 'hidden',
    '#value' => $media->fid,
  );

  return $form;
}

function media_edit_validate($form, &$form_state) {
  $media = media_load($form_state['values']['fid']);
  field_attach_form_validate('media', $media, $form, $form_state);
}

/**
 * Process media_edit form submissions.
 */
function media_edit_submit($form, &$form_state) {
  $media = media_load($form_state['values']['fid']);
  field_attach_submit('media', $media, $form, $form_state);
  media_save($media);
}

function media_edit_delete_submit($form, &$form_state) {
  $fid = $form_state['values']['fid'];
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $form_state['redirect'] = array('media/' . $fid . '/delete', array('query' => $destination));
}

function media_add_remote($form, &$form_state) {
  // Totally prototyping code to show designs.
  $form['sources'] = array(
    '#type' => 'vertical_tabs',
    '#title' => 'Sources',
  );

  $form['sources']['paste'] = array(
    '#type' => 'fieldset',
    '#title' => 'Paste URL or embed code',
  );


  $providers = '';
  $providers .= '<img style="height:50px; margin:20px" src="http://www.macuser.com/2008/10/09/top_youtube_logo_31_Dec_06.jpg">';
  $providers .= '<img style="height:50px; margin:20px" src="http://jasonhilldesign.com/FlikrLogo.jpg">';
  $form['sources']['paste']['code'] = array(
    '#type' => 'textarea',
    '#title' => t('URL or embed code'),
    '#description' => t('The following providers are supported: <br/>' . $providers),
  );

  $form['sources']['youtube'] = array(
    '#type' => 'fieldset',
    '#title' => 'YouTube',
    '#description' => t(''),
    '#attributes' => array('style' => 'height: 300px; overflow:auto'),
  );

  $form['sources']['flikr'] = array(
    '#type' => 'fieldset',
    '#title' => 'Flikr',
  );

  $box = '<div style="width:100px; height:100px; border:1px solid blue; padding:10px; float:left; margin:5px;"> Video </div>';
  $boxes = '';
  for ($i = 0; $i < 10; $i++) {
    $boxes .= $box;
  }
  $form['sources']['youtube']['stupid'] = array(
    '#markup' => $boxes,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#attributes' => array('style' => 'float:right'),
    '#suffix' => '<br style="clear:both" />',
  );

  return $form;
}

