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
 * Menu callback; presents the Media editing form, or redirects to delete confirmation.
 */
function media_page_edit($media) {
  drupal_set_title(t('Edit @title', array('@title' => drupal_set_title($media->filename))));
  return drupal_get_form('media_edit', $media);
}

/**
 * Menu callback; shows delete confirmation form.
 */
function media_page_delete($media) {
  $files = array($media);
  return drupal_get_form('media_multiple_delete_confirm', $files);
}

/**
 * Form callback for adding media via an upload form.
 * @todo: should use the AJAX uploader
 */
function media_add_upload($form, &$form_state, $redirect = NULL) {

  $validators = array(
   'file_validate_extensions' => array(media_variable_get('file_extensions')),
   'file_validate_size' => array(parse_size(media_variable_get('max_filesize'))),
  );


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

  $form['redirect'] = array(
    '#type' => 'value',
    '#value' => $redirect,
  );

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
  }
  if (empty($form_state['values']['redirect'])) {
    $form_state['redirect'] = 'media/' . $file->fid . '/edit';
  }
  else {
    // Eek, this is a hack isn't it...
    if ($form_state['values']['redirect'][0] = 'media/browser' && $file->fid) {
      $form_state['redirect'] = array('media/browser', array('query' => array('render' => 'media-popup', 'fid' => $file->fid)));
    }
  }
}


/**
 *  Callback for /media/add/upload and /admin/content/media/add/upload.
 *  @TODO: work on the description.
 *  @TODO: allow stream wrappers to add to the description.
 *  @see media_media_browser_plugins().
 */
function media_add_from_url($form, &$form_state = array(), $redirect = NULL) {
  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => 'URL',
    '#description' => 'Input any url which Drupal can handle',
    '#attributes' => array('class' => array('media-add-from-url')),
  );

  $form['redirect'] = array(
    '#type' => 'value',
    '#value' => $redirect,
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

  if (empty($uri)) {
    // @todo - at the moment we don't actual do any real validation.
    // form_set_error('url', t('Invalid URL'));
  }
  // @TODO: Validate that if we have no $uri that this is a valid file to
  // save. For instance, we may only be interested in images, and it would
  // be helpful to let the user know they passed the HTML page containing
  // the image accidentally. That would also save us from saving the file
  // in the submit step.
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

  if (!empty($form_state['values']['redirect'])) {
    $form_state['redirect'] = $form_state['values']['redirect'];
  }

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

  if (!$form_state['redirect']) {
    $form_state['redirect'] = 'media/' . $file->fid . '/edit';
  }
  else {
    if ($form_state['values']['redirect'][0] = 'media/browser' && $file->fid) {
      $form_state['redirect'] = array('media/browser', array('query' => array('render' => 'media-popup', 'fid' => $file->fid)));
//       $form_state['redirect'] = array('media/'. $file->fid .'/format-form', array('query' => array('render' => 'media-popup')));
    }
  }
}

/**
 * Form builder: Builds the edit file form.
 */
function media_edit($form, $form_state, $media) {
  drupal_set_title($media->filename);
  field_attach_form('media', $media, $form, $form_state);
  unset($form['file']);

//   $form['#attached'] = media_attached_files();

  $form['preview'] = field_view_field('media', $media, 'file', 'media_preview');
  $form['preview']['#weight'] = -10;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100,
  );

  $form['fid'] = array(
    '#type' => 'hidden',
    '#value' => $media->fid,
  );

  $form['#redirect'] = 'admin/content/media';
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

