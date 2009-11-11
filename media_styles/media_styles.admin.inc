<?php
// $Id$

/**
 *  @file
 *  Administrative page callbacks for the Media: Styles module.
 */

/**
 * Menu callback; Listing of all current Media styles.
 */
function media_style_list() {
  $page = array();

  $styles = media_styles();
  $page['media_style_list'] = array(
    '#markup' => theme('media_style_list', array('styles' => $styles)),
    '#attached' => array(
      'css' => array(drupal_get_path('module', 'media_styles') . '/media_styles.admin.css' => array('preprocess' => FALSE)),
    ),
  );

  return $page;
}

/**
 * Form builder; Form for adding a new media style.
 *
 * @ingroup forms
 * @see media_style_add_form_submit()
 * @see media_style_name_validate()
 */
function media_style_add_form($form, &$form_state) {
  $form['name'] = array(
    '#type' => 'textfield',
    '#size' => '64',
    '#title' => t('Style name'),
    '#default_value' => '',
    '#description' => t('The name is used in URLs for generated images and the like. Use only lowercase alphanumeric characters, underscores (_), and hyphens (-).'),
    '#element_validate' => array('media_style_name_validate'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create new style'),
  );

  return $form;
}

/**
 * Submit handler for adding a new image style.
 */
function media_style_add_form_submit($form, &$form_state) {
  $style = array('name' => $form_state['values']['name']);
  $style = media_style_save($style);
  drupal_set_message(t('Style %name was created.', array('%name' => $style['name'])));
  $form_state['redirect'] = 'admin/config/media/media-styles/edit/' . $style['name'];
}

/**
 * Element validate function to ensure unique, URL safe style names.
 */
function media_style_name_validate($element, $form_state) {
  // Check for duplicates.
  $styles = media_styles();
  if (isset($styles[$element['#value']]) && (!isset($form_state['media_style']['msid']) || $styles[$element['#value']]['msid'] != $form_state['media_style']['msid'])) {
    form_set_error($element['#name'], t('The media style name %name is already in use.', array('%name' => $element['#value'])));
  }

  // Check for illegal characters in image style names.
  if (preg_match('/[^0-9a-z_\-]/', $element['#value'])) {
    form_set_error($element['#name'], t('Please only use lowercase alphanumeric characters, underscores (_), and hyphens (-) for style names.'));
  }
}

function media_style_form() {
}

function media_style_delete_form() {
}

function media_style_revert_form() {
}

function media_effect_form() {
}

function media_effect_delete_form() {
}

// function media_effect_form() {
// }

/**
 * Display the page containing the list of media styles.
 *
 * @param $variables
 *   An associative array containing:
 *   - styles: An array of all the media styles returned by media_get_styles().
 *
 * @see media_get_styles()
 * @ingroup themeable
 */
function theme_media_style_list($variables) {
  $styles = $variables['styles'];

  $header = array(t('Style name'), t('Settings'), array('data' => t('Operations'), 'colspan' => 3));
  $rows = array();
  foreach ($styles as $style) {
    $row = array();
    $row[] = l($style['name'], 'admin/config/media/media-styles/edit/' . $style['name']);
    $link_attributes = array(
      'attributes' => array(
        'class' => array('image-style-link'),
      ),
    );
    if ($style['storage'] == IMAGE_STORAGE_NORMAL) {
      $row[] = t('Custom');
      $row[] = l(t('edit'), 'admin/config/media/media-styles/edit/' . $style['name'], $link_attributes);
      $row[] = l(t('delete'), 'admin/config/media/media-styles/delete/' . $style['name'], $link_attributes);
    }
    elseif ($style['storage'] == IMAGE_STORAGE_OVERRIDE) {
      $row[] = t('Overridden');
      $row[] = l(t('edit'), 'admin/config/media/media-styles/edit/' . $style['name'], $link_attributes);
      $row[] = l(t('revert'), 'admin/config/media/media-styles/revert/' . $style['name'], $link_attributes);
    }
    else {
      $row[] = t('Default');
      $row[] = l(t('edit'), 'admin/config/media/media-styles/edit/' . $style['name'], $link_attributes);
      $row[] = '';
    }
    $rows[] = $row;
  }

  if (empty($rows)) {
    $rows[] = array(array(
      'colspan' => 4,
      'data' => t('There are currently no styles. <a href="!url">Add a new one</a>.', array('!url' => url('admin/config/media/media-styles/add'))),
    ));
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}