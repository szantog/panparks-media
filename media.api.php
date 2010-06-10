<?php
// $Id$

/**
 * Return an array of plugins for the media browser.
 *
 * Implementors are expected to return a renderable element.
 *
 * Each element will be a jQuery tab on the media browser.
 *
 * Some elements are special:
 *  - #title: The title that goes on the tab
 *  - #settings: Drupal.settings.media.browser.$key (where key is the array key).
 *  - #callback: If provided, will make the tab an "ajax" tab.
 *
 * Example:
 *   $plugins['library'] = array(
 *  '#title' => t('Library'),
 *  '#attached' => array(
 *    'js' => array(
 *       $path . '/javascript/plugins/media.library.js',
 *     ),
 *   ),
 *   '#settings' => array(
 *     'viewMode' => 'thumbnails',
 *     'getMediaUrl' => url('media/browser/list'),
 *   ),
 *   '#markup' => '<div> Library goes here</div>',
 * );
 *
 * @param $plugin_name
 *  The name of the plugin to view
 *
 * @param $params
 *  An array of parameters which came in is $_GET['params'].
 *  The expected parameters is still being defined.
 *   - types: Array of media types to support
 *
 * @return
 *  Renderable array.
 */
function hook_media_browser_plugin_view($plugin_name, $params) {

}

/**
 * Returns a list of plugins for the media browser.
 *
 * Plugins are defined in a multi-dimensional associative
 * array format with the following keys:
 *
 * - #weight (optional): Weight of the plugin in relation to other plugins
 *  when being displayed, e.g. tabs in the browser.
 *
 * @example
 * <code>
 * array(
 *  'unique_plugin_name' => array(
 *     '#weight' => 42,
 *   ),
 * );
 * </code>
 */
function hook_media_browser_plugin_info() {
  
}