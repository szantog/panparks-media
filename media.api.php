<?php
// $Id$

/**
 * Return an array of plugins for the media browser.
 *
 * Implementors are expected to return a keyed array of renderable elements.
 *
 * Each element will be a jQuery tab on the media browser (we should make this more flexible).
 * May be replaced if tabs module gets upgraded.
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
 * @param $params
 *  An array of parameters which came in is $_GET['params'].
 *  The expected parameters is still being defined.
 *   - types: Array of media types to support
 *
 * @return
 *  Renderable array.
 */
function hook_media_browser_plugins($params) {

}