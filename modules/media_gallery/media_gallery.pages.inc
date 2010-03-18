<?php
// $Id;

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

  // For multi-step processing and Add more widget...
  // Weird.  related to builder_function.
  // @see media_gallery_form_submit_build_gallery
  if (isset($form_state['gallery'])) {
    foreach ($form_state['gallery'] as $k => $v) {
      $gallery->{$k} = $v;
    }
  }
  $form['#builder_function'] = 'media_gallery_form_submit_build_gallery';
  field_attach_form('gallery', $gallery, $form, $form_state);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $is_edit ? 'Update' : 'Create',
    '#weight' => 10,
  );

  if ($is_edit) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => 'Delete',
      '#weight' => 10,
    );
  }

  return $form;
}

function media_gallery_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == 'Delete') {
    drupal_set_message('Sorry, delete is not yet implemented.');
    return;
  }
  
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
  $form_state['redirect'] = 'gallery/' . $g->gid . '/edit';
}

function media_gallery_view($gallery, $view_mode = 'slideshow') {
  if (!$gallery) {
    return drupal_not_found();
  }


  field_attach_prepare_view('gallery', array($gallery->gid => $gallery));
  entity_prepare_view('gallery', array($gallery->gid => $gallery));
  return field_attach_view('gallery', $gallery, $view_mode);

}