<?php

/** Callback **/
function media_gallery_admin_list() {


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

  if (isset($form_state['values']['gid'])) {
    $g = media_gallery_load($form_state['values']['gid']);
  }
  else {
    $g = new Gallery();
  }
  field_attach_submit('gallery', $g, $form, $form_state);
  field_attach_presave('gallery', $g);
  $g->save();
}