<?php
// $Id$

/**
 * @file
 * Experimental browser, testing alternative FAPI integration with the popup.
 *
 * To use this, you'll need to add the following variable override to your
 * settings.php:
 *   $conf['media_experimental_browser'] = TRUE;
 */

function media_browser_experimental() {
  return drupal_get_form('media_browser_form', media_browser_fid_list());
}

/**
 * Build a form listing available media files.
 *
 * @param array $form
 *  The form built with FAPI.
 * @param array &$form_state
 *  The current form state after rendering.
 * @param array $selections
 *  An array of thumbnails associated by fid or uri for the radio selection
 *  options.
 */
function media_browser_form($form, &$form_state, $selections = array()) {
  $form = array();

  $form['#attached']['library'][] = array('media', 'media_browser_page');

  $form['media'] = array(
    '#type' => 'radios',
    '#options' => $selections,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('OK'),
  );

  return $form;
}

function media_browser_form_submit(&$form, &$form_state) {
  $media = $form_state['values']['media'];

  if (is_numeric($media)) {
    $url = url('media/'. $media. '/format-form', array('query' => array('render' => 'media-popup'), 'absolute' => TRUE));
    $form_state['redirect'] = $url;
  }
}

/**
 *  Any options needed to retrieve the file select options. An associate array
 *  with the following optional keys:
 *    'conditions' => An array of 'column' => 'value' to pass to the db query,
 *    'streams' => An array of streams to filter,
 *    'limit' => The number of results to return.
 */
function media_browser_fid_list($options = array()) {
  $conditions = isset($options['conditions']) ? $options['conditions'] : array();
  $streams = isset($options['streams']) ? $options['streams'] : array();
  $limit = isset($options['limit']) ? $options['limit'] : media_variable_get('browser_pager_limit');

  // First get the fid's to load. We have to do that first, because
  // entity_load doesn't accept a condition of LIKE, which we need for streams.
  $select = db_select('file', 'f')
    ->extend('PagerDefault')
    ->fields('f', array('fid'));

  // Filter on streams.
  foreach ($streams as $stream) {
    $select->condition('uri', db_like($stream) . '%', 'LIKE');
  }

  // Add our conditions.
  foreach ($conditions as $field => $condition) {
    $select->condition($field, $condition);
  }

  // Add our pager limit filter.
  $select->limit($limit);

  // Grab the uri's.
  $fids = $select->execute()
    ->fetchCol();

  $selections = array();

   if (!empty($fids)) {
    // Now load the desired media to display.
    $medias = entity_load('media', $fids);

    foreach ($medias as &$media) {
      $media->override = array('browser' => TRUE);
      // Generate a preview of the file
      // @todo: Should generate placeholders for audio
      // Add yet another wrapper so we can style it as a preview :(
      // Otherwise it isn't really possible to know because the user can pick anything for their preview mode.
      $preview = field_view_field('media', $media, 'file', 'media_preview');
      $preview['#show_names'] = TRUE;
      $preview['#theme_wrappers'][] = 'media_thumbnail';

      $selections[$media->fid] = drupal_render($preview);
    }
  }

  return $selections;
}
