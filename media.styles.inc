<?php
// $Id$

/**
 *  @file
 *  Media widget formatter definitions.
 */

/**
 * Media constant for user style bundles in the database.
 */
define('MEDIA_STORAGE_NORMAL', 1);

/**
 * Media constant for user style bundles that override module-defined bundles.
 */
define('MEDIA_STORAGE_OVERRIDE', 2);

/**
 * Media constant for module-defined bundles in code.
 */
define('MEDIA_STORAGE_DEFAULT', 4);

/**
 * Media constant to represent an editable preset.
 */
define('MEDIA_STORAGE_EDITABLE', MEDIA_STORAGE_NORMAL | MEDIA_STORAGE_OVERRIDE);

/**
 * Media constant to represent any module-based preset.
 */
define('MEDIA_STORAGE_MODULE', MEDIA_STORAGE_OVERRIDE | MEDIA_STORAGE_DEFAULT);

/**
 *  Implement hook_field_formatter_info().
 */
function media_field_formatter_info() {
  $styles = array();

  // Each Media Style bundle becomes a new formatter for File (media) fields.
  foreach (media_style_bundles() as $bundle_name => $bundle) {
    $styles[$bundle_name] = array(
      'label' => $bundle['label'],
      'field types' => array('media_generic', 'file'),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
      ),
    );
  }

  return $styles;
}

/**
 *  Build a list of all registered Media style bundles.
 *
 *  Each style within a bundle will be attached to a specific mimetype, and
 *  will be invoked as required when displaying a file field instance.
 *
 *  To register a new display formatter for a specific mimetype to be made
 *  available to the system, you will need to implement
 *  hook_media_styles($mimetype).
 *
 *  To register a default, module-defined bundle of Media styles, you will
 *  need to implement hook_media_default_style_bundles().
 *
 *  See API.txt for more information.
 *
 *  @return
 *    This returns an array of all media style bundles, keyed by the style
 *    bundle names.
 */
function media_style_bundles() {
  $styles = &drupal_static(__FUNCTION__);

  // Grab from cache or build the array.
  if (!isset($styles)) {
    if ($cache = cache_get('media_style_bundles', 'cache')) {
      $styles = $cache->data;
    }
    else {
      $styles = array();

      // Select the module-defined styles.
      foreach (module_implements('media_default_style_bundles') as $module) {
        $module_styles = module_invoke($module, 'media_default_style_bundles');
        foreach ($module_styles as $style_name => $style) {
          $style['name'] = $style_name;
          $style['module'] = $module;
          $style['storage'] = MEDIA_STORAGE_DEFAULT;
          $styles[$style_name] = $style;
        }
      }

      // Select all the user-defined styles.
      $user_styles = db_select('media_style_bundles', NULL, array('fetch' => PDO::FETCH_ASSOC))
        ->fields('media_style_bundles')
        ->orderBy('name')
        ->execute()
        ->fetchAllAssoc('name', PDO::FETCH_ASSOC);

      // Allow the user styles to override the module styles.
      foreach ($user_styles as $style_name => $style) {
        $style['module'] = NULL;
        $style['storage'] = MEDIA_STORAGE_NORMAL;

        // Formatters will be an array of module-defined formatters keyed by
        // mimetype.
        $style['formatters'] = unserialize($style['formatters']);
        if (isset($styles[$style_name]['module'])) {
          $style['module'] = $styles[$style_name]['module'];
          $style['storage'] = MEDIA_STORAGE_OVERRIDE;
        }
        $styles[$style_name] = $style;
      }

      // Allow any module implementing hook_media_style_bundles to alter the styles.
      drupal_alter('media_style_bundles', $styles);
      cache_set('media_style_bundles', $styles);
    }
  }

  return $styles;
}
