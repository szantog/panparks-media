<?php
// $Id$

/**
 * List all galleries.
 */
function media_gallery_admin_list($form, &$form_state) {
  $options = array();
  $empty_message = t('You have no galleries. Click "Create gallery" to create one.');

  $gids = db_select('gallery', 'g')
    ->fields('g', array('gid'))
    ->execute()
    ->fetchCol();
  if (!$gids) {
    $form['message'] = array(
      '#markup' => $empty_message, 
    );
    return $form;
  }
  $galleries = entity_get_controller('gallery')->load($gids);
  foreach ($galleries as $gallery) {
    $options[$gallery->gid] = array(
      l($gallery->field_gallery_title[LANGUAGE_NONE][0]['safe_value'], 'admin/content/gallery/' . $gallery->gid . '/edit'),
      count($gallery->field_gallery_item[LANGUAGE_NONE]),
      l(t('Embed in block'), 'admin/content/gallery/' . $gallery->gid . '/embed') . ' | ' . l(t('Delete (not yet working)'), 'admin/content/gallery/' . $gallery->gid . '/delete'),
    );
  }

  $form['galleries'] = array (
    '#type' => 'tableselect',
    '#header' => array('Title', '# of Items', 'Operations'),
    '#options' => $options,
    '#title' => 'Galleries',
  );
  
  return $form;
  
}

function media_gallery_admin_settings($form, &$form_state) {
  $form['temp'] = array(
    '#markup' => t('To change the fields and display settings for all Galleries, use the "Manage fields" and "Manage display" tabs (above). (This will be a settings for one day)')
  );
  return $form;
}

/**
 * Menu Callback: Creates a block for the provided gallery
 * and redirects the user to the settings page for that block.
 * @param <type> $gallery
 */
function media_gallery_embed($gallery) {
  $delta = media_gallery_create_block($gallery->gid);
  drupal_goto('/admin/structure/block/manage/media_gallery/' . $delta . '/configure');
  return $delta;
  // Create a block for this thing... Man this sucks...
  
}

/**
 * Creates a block for the provided gallery ID and returns its delta
 *
 * Since a single gallery may be used in multiple gallery blocks, this function
 * assigns a random(ish) delta to it.  This isn't pretty, but neither is enummerating
 * and assigning, so I guess it is good enough for now.
 *
 * @param int $gid
 * @return string
 *  The delta of the block
 */
function media_gallery_create_block($gid) {
  $delta = uniqid('mg_');

  db_insert('gallery_blocks')
    ->fields(array('gid' => $gid, 'delta' => $delta))
    ->execute();
  
}