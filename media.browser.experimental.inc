<?php
// $Id$

/**
 * @file
 * Experimental browser, testing alternative FAPI integration with the popup.
 *
 * To use this, you'll need to add the following variable override to your
 * settings.php:
 *   $conf['experimental_browser'] = TRUE;
 */

/**
 * Build a form listing available media files.
 *
 * @param array $form
 *  The form built with FAPI.
 * @param array &$form_state
 *  The current form state after rendering.
 * @param array $options
 *  Any options needed to retrieve the file select options. An associate array
 *  with the following optional keys:
 *    'conditions' => An array of 'column' => 'value' to pass to the db query,
 *    'streams' => An array of streams to filter,
 *    'limit' => The number of results to return.
 */
function media_browser_form($form, &$form_state, $options = array()) {
  $form = array();

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

  // Grab the fid's.
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

  $form['media'] = array(
    '#type' => 'radios',
    '#options' => $selections,
  );

  return $form;
}
