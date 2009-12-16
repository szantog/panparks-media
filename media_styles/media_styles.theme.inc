<?php
// $Id$

/**
 *  @file
 *  Theme functions for Media styles.
 */

/**
 *  A wrapper for the image style preview.
 */
function theme_media_styles_image_style_preview($variables) {
  $style_name = $variables['style_name'];
  $styles = image_styles();
  if (isset($styles[$style_name])) {
    // The theme_image_style_preview function is in image.admin,
    // but not properly referenced in hook_theme.
    // @TODO: File a bug report and fix this in core.
    module_load_include('inc', 'image', 'image.admin');
    drupal_add_css(drupal_get_path('module', 'image') . '/image.admin.css');

    // Ensure we revert the module & storage namespace for the Image module.
    $styles[$style_name]['module'] = $styles[$style_name]['image_module'];
    $styles[$style_name]['storage'] = $styles[$style_name]['image_storage'];

    return theme('image_style_preview', array('style' => $styles[$style_name]));
  }
}

function theme_media_styles_styles_default($variables) {
  $file = $variables['object'];
  $path = drupal_get_path('module', 'media');
  $thumbnail = theme('image',
    array(
      'path' => $path . '/images/file-unknown.png',
      'alt' => t('Thumbnail for !filename.', array('!filename' => $file->filename)),
      'attributes' => array('class' => 'file-unknown'),
    )
  );
  return $thumbnail;
}

function theme_media_styles_preview_default($variables) {
  return 'default: theme_media_styles_preview_default';
}

function theme_media_styles_field_formatter_styles_default($variables) {
  $file = $variables['object'];
  $file_url = file_create_url($file->uri);
  return l($file->filename, $file_url);
}

function theme_media_styles_field_formatter_styles($variables) {
  // The formatter name needs to match what theme_field_formatter_image expects.
  $variables['element']['#formatter'] = '__' . $variables['style']['name'];

  // Add Alt & Title to the element.
  $variables['element']['#item']['alt'] = $variables['element']['#item']['title'] = $variables['element']['#item']['description'];

  // Defer to Image Styles for the rest.
  return theme('field_formatter_image', $variables);
}

function theme_media_styles_styles($variables) {
  $style_name = $variables['style_name'];
  $path = $variables['object']->uri;
  $alt = $title = $variables['object']->description;
  return theme('image_style',
    array(
      'style_name' => $style_name,
      'path' => $path,
      'alt' => $alt,
      'title' => $title,
    )
  );
}
