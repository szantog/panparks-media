<?php
// $Id;

/**
 * List all galleries.
 */
function media_gallery_admin_list($form, &$form_state) {
  $gids = db_select('gallery', 'g')
    ->fields('g', array('gid'))
    ->execute()
    ->fetchCol();
  $galleries = entity_get_controller('gallery')->load($gids);
  foreach ($galleries as $gallery) {
    $options[$gallery->gid] = array(
      l($gallery->field_gallery_title[LANGUAGE_NONE][0]['safe_value'], 'admin/content/gallery/' . $gallery->gid . '/edit'),
      count($gallery->field_gallery_title[LANGUAGE_NONE]),
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

function media_gallery_blah() {
  return "nothing here, just for the menu";
}

function media_gallery_form($form, &$form_state, $gallery = NULL) {
  $is_edit = FALSE;
  // IS new?
  if (!$gallery){
    $gallery = new Gallery();
  }
  else {
    $is_edit = TRUE;
    $form['gid'] = array(
      '#type' => 'value',
      '#value' => $gallery->gid,
    );
  }
  
  field_attach_form('gallery', $gallery, $form, $form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $is_edit ? 'Update' : 'Create',
    '#weight' => 10,
  );

  return $form;
}

function media_gallery_form_submit($form, &$form_state) {
  $is_edit = FALSE;

  if (isset($form_state['values']['gid'])) {
    $is_edit = TRUE;
    $g = media_gallery_load($form_state['values']['gid']);
  }
  else {
    $g = new Gallery();
  }

  field_attach_submit('gallery', $g, $form, $form_state);
  field_attach_presave('gallery', $g);
  $g->save();
  $form_state['redirect'] = $is_edit ? '' : 'gallery/' . $g->gid . '/edit';
}