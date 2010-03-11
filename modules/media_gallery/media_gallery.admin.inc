<?php

/** Callback **/
function media_gallery_admin_list() {


}

function media_gallery_blah() {
  return "nothing here, just for the menu";
}

function media_gallery_form($form, &$form_state, $gallery = NULL) {
  // IS new?
  if (!$gallery){
    $gallery = new Gallery();
  }
  else {
    $form['gid'] = array(
      '#type' => 'value',
      '#value' => $gallery->gid,
    );
  }
  
  field_attach_form('gallery', $gallery, $form, $form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Create',
    '#weight' => 10,
  );

  return $form;
}

function media_gallery_form_submit($form, &$form_state) {

  if (isset($form_state['values']['gid'])) {
    $g = new Gallery(array('gid' => $form_state['values']['gid']));
  }
  else {
    $g = new Gallery();
  }
  field_attach_submit('gallery', $g, $form, $form_state);
  field_attach_presave('gallery', $g);
  $g->save();

  //$form_state['redirect'] = 'gallery/' . $x . '/edit';
}