<?php
// $Id$

/**
 * @file
 * Cruft that might be useful but isn't being used.
 */

/**
 * Build data for the media browser display.
 *
 * @TODO Pick what we can from this and remove.
 *
 * Note: The FAPI docs say a submit element event defaults to 'click',
 * but as of d6.10, it defaults to 'mousedown', so we need to override.
 *   'event' => 'click',
 *
 * @param array $registration_ids
 *   Array of registrations to call.
 * @param string $node_type
 * @param string $field
 * @param uid $uid
 * @return array
 *   Drupal FAPI form array.
 */
function media_build_browser_form($form_state, $registration_ids, $node_type, $field, $uid) {
  static $id;

  // We need a static counter for our form element wrapper.
  $id += 1;

  // Load our css.
  $path = drupal_get_path('module', 'media');
  drupal_add_css($path .'/media.css');
  // Load our specific js for the file selector
  drupal_add_js($path .'/javascript/media.js');
  // Load the md5 library so we can hash the upload filename for use in the meta form.
  drupal_add_js($path .'/javascript/jquery.md5.js');

  $items = array();
  $form  = array();

  $form['media_browser_activate'] = array(
    '#type'  => 'markup',
    '#value' => '<div class="media browser activation">'. t('Add files') .'</div>',
  );

  // We are using a tab form type.
  $form['media_browser'] = array(
    '#type'       => 'tabset',
    '#attributes' => array('class' => 'media browser wrapper'),
  );

  // Get all the active resources
  $resources = media_get_resources($registration_ids, $node_type, $field, $uid);

  // Store the tab & drawer names for the js form selector.
  $drawer_options = array();

  // loop through the form and start pulling out the data to
  // create tabs -> panes -> drawers
  foreach ($resources as $tab_name => $data) {
    // create a tab id
    $tab_id = strtolower(str_replace(' ', '_', $tab_name));
    // create tab
    $form['media_browser'][$tab_id] = array(
      '#type'  => 'tabpage',
      '#title' => $tab_name,
      '#theme' => 'media_browser_pane',
    );

    $drawer_options[$tab_id] = $tab_name;

    // build the drawers for this tab
    $drawer_list   = array();
    $active_drawer = TRUE;

    // check to see if we do have children- we should, but just in case
    if (is_array($data)) {
      $drawers = array();

      foreach ($data as $drawer_name => $drawer_data) {
        // @TODO check drawer access permissions here to make sure we should present this to the user

        // The drawer id needs to have additional data on it to prevent
        // name space conflicts with ids
        $drawer_id = strtolower(str_replace(' ', '_', $drawer_name)) .'_display';

        // create a link with a specific id to call
        $drawers_link  = '<a onclick="javascript: return FALSE;" href="#'. $drawer_id .'">'. $drawer_name .'</a>';
        $drawer_list[] = array('data' => $drawers_link, 'class' =>  ($active_drawer ? 'active' : '') );

        // add the drawer form element
        $form['media_browser'][$tab_id][$drawer_name] = $drawer_data;

        // add classes to the drawer display item
        $form['media_browser'][$tab_id][$drawer_name]['#prefix'] = '<div id="'. $drawer_id .'" class="drawer display '. ($active_drawer ? ' active' : NULL) .'">';
        $form['media_browser'][$tab_id][$drawer_name]['#suffix'] = '</div>';

        // no longer on the first drawer
        $active_drawer = FALSE;

        $drawer_options[$tab_id .'|'. $drawer_id] = '- '. $drawer_name;
      }

      // change the drawers to a list for easer display
      $form['media_browser'][$tab_id]['drawers'] = array(
        '#type'  => 'markup',
        '#value' => theme('item_list', $drawer_list, NULL, 'ul', array('class' => 'drawers'))
      );

    }
  }

  $form['media_browser']['drawer_select'] = array(
    '#type'        => 'select',
    '#title'       => t('Drawer select'),
    '#description' => t('Oh bother, you really should have JavaScript enabled, you know...'),
    '#options'     => $drawer_options,
    '#prefix'      => '<div class="media-browser-drawer-select">',
    '#suffix'      => '</div>',
  );

  // Container for the progress indicator.
  $form['media_browser']['media_browser_file_progress'] = array(
    '#prefix' => '<div id="media-browser-file-progress-'. $id .'" class="media-browser-file-progress">',
    '#suffix' => '</div>',
  );
  $form['media_browser']['media_browser_file_progress']['file_progress_message'] = array(
    '#type'  => 'item',
    '#title' => theme('image', media_variable_get('file_progress_image', $path .'/images/uploading-gradient.gif')) . t('Please wait while your file is attached...'),
  );

  // AHAH enabled submit button.
  $form['media_browser']['media_browser_submit'] = array(
    '#type'        => 'submit',
    '#value'       => t('Add file'),
    '#description' => t("Add the selected file."),
    '#submit'      => array('media_browser_submit'), // If no javascript action.
    '#validate'    => array('media_browser_validate'),
    '#attributes'  => array('class' => 'media-browser-submit'),
    '#ahah'        => array(
      'path'         => 'media/js',
      'wrapper'      => 'media-browser-file-progress-'. $id,
      'method'       => 'replace',
      'effect'       => 'fade',
      'event'        => 'click',
    ),
  );

  // Container for the metadata submission message.
  $form['media_browser']['media_browser_metadata_message'] = array(
    '#prefix' => '<div id="media-browser-metadata-message-'. $id .'" class="media-browser-metadata-message">',
    '#suffix' => '</div>',
  );
  $form['media_browser']['media_browser_metadata_message']['message'] = array(
    '#type'  => 'item',
    '#value' => '',
  );

  $form['media_browser']['media_browser_metadata'] = array(
    '#type'       => 'tabset',
    '#attributes' => array('class' => 'media-browser-metadata-wrapper'),
  );

  // @TODO: This all goes in the form creation, actually,
  // to create our metadata form...
  /*
  $uri = $form_state['values']['media_files'];
  $file_extension = pathinfo($uri, PATHINFO_EXTENSION);

  // Get the file creator for this item.
  $file_creator = media_get_registered_modules(array($registration_id));

  // Get the formaters for this node type.
  $formatters = media_active_fields_for_node_type($node_type, 'formatter');

  // Get the registrations.
  $registrations = media_get_registered_modules($formatters[$field]);

  // Remove any non-applying registrations.
  $registrations = media_get_applicable_formatters($registrations, $file_extension);

  // Get all the formatting forms.
  $formatter_options = array();
  $forms = array();
  foreach ($registrations as $id => $registration) {
    $formatter_options[$id] = $registration['name'];
    $function = $registration['callbacks']['form'];
    if (function_exists($function)) {
      $forms[$id] = $function($node_type, $field, $file_extension, $uri);
    }
  }
  */

  // @TODO: This is placeholder only.
  foreach (array('Video', 'Image', 'Audio', 'PDF') as $mime_type) {
    $form['media_browser']['media_browser_metadata'][$mime_type] = array(
      '#type'  => 'tabpage',
      '#title' => $mime_type,
    );
    $form['media_browser']['media_browser_metadata'][$mime_type][$mime_type .'_title'] = array(
      '#type'  => 'textfield',
      '#title' => t('Title'),
    );
  }

  // Our AHAH enabled submit button for the metadata.
  $form['media_browser']['media_browser_metadata_submit'] = array(
    '#type'        => 'submit',
    '#value'       => t('Add metadata'),
    '#description' => t("Add the selected file."),
    '#submit'      => array('media_browser_metadata_submit'), // If no javascript action.
    '#validate'    => array('media_browser_metadata_validate'),
    '#attributes'  => array('class' => 'media-browser-metadata-submit'),
    '#ahah'        => array(
      'path'         => 'media/metadata/js',
      'wrapper'      => 'media-browser-metadata-message-'. $id,
      'method'       => 'replace',
      'effect'       => 'fade',
      'event'        => 'click',
    ),
  );

  // build the tabs into a single form element
  // @TODO make sure we have children for each tab and remove any that
  //       we don't have data for
  $form['tabs'] = array(
    '#type'  => 'markup',
    '#value' => theme('item_list', $tabs, NULL, 'ul', array('class' => 'tabs')),
  );

  return $form;
}