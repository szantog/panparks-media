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
    );
  }

  $form['galleries'] = array (
    '#type' => 'tableselect',
    '#header' => array('Title', '# of Items'),
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