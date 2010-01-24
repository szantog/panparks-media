<?php

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
  $uri = media_parse($url);
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
 * Form builder: Builds the edit file form.
 */
function media_edit($form, $form_state, $media) {
  drupal_set_title($media->filename);
  field_attach_form('media', $media, $form, $form_state);

  $form['#attached'] = media_attached_files();

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
