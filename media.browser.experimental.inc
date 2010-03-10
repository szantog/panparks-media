<?php
// $Id$

/**
 * @file
 * Experimental browser, testing alternative architechture.
 *
 * To use this, you'll need to add the following variable override to your
 * settings.php:
 *   $conf['media__experimental_browser'] = TRUE;
 */
function media_browser_experimental($selected = NULL) {
  $output = array();
  $output['#attached']['library'][] = array('media', 'media_browser_page_experimental');

  $get = drupal_get_query_parameters();
  array_walk_recursive($get, '_media_recursive_check_plain');

  // We don't want everything, just this array.
  $params = isset($get['params']) ? $get['params'] : array();

  // How do we validate these?  I don't know.
  // I think PDO should protect them, but I'm not 100% certain.
  
  
  if (isset($get['fid'])) {
    // A form has submitted, selected a piece (or pieces) of media and we refreshed.
    // So we're just going to send the media entity out as a JS setting so the browser can close itself.
    if (!is_array($get['fid'])) {
      $selected = array($get['fid']);
    }
    $selectedMedia = entity_load('media', $selected);
    $setting = array('media' => array('selectedMedia' => array_values($selectedMedia)));
    drupal_add_js($setting, 'setting');
    return $output;
  }

  // Normal browser operation.
  

  // See hook_media_browser_new_plugins
  $plugins = module_invoke_all('media_browser_new_plugins', $params);

  $tabs = array(); // List of tabs to render.

  $settings = array('media' => array('browser' => array()));
  $browser_settings =& $settings['media']['browser'];

  //@todo: replace with Tabs module if it gets upgraded.
  foreach (element_children($plugins) as $key) {
    $plugin =& $plugins[$key];

    //Add any JS settings
    $browser_settings[$key] = isset($plugin['#settings']) ? $plugin['#settings'] : array();

    // If this is a "ajax" style tab, add the href, otherwise an id.
    $href = isset($plugin['#callback']) ? $plugin['#callback'] : "#media-tab-$key";
    $tabs[] = "<a href='$href'><span>{$plugin['#title']}</span></a>";

    // Create a div for each tab's content.
    $plugin['#prefix'] = <<<EOS
    <div class="media-browser-tab" id="media-tab-$key">
EOS;
    $plugin['#suffix'] = <<<EOS
    </div>
    <!-- End #media-tab-$key -->
EOS;
  }
  
  drupal_add_js($settings, 'setting');

  $output['tabset'] = array(
    '#prefix' => '<div id="media-browser-tabset">',
    '#suffix' => '</div>',
  );
  $output['tabset']['list'] = array(
    '#markup' => '<ul><li>' . implode('</li><li>', $tabs) . '</li></ul>'
  );
  
  $output['tabset']['plugins'] = $plugins;
  return $output;
}

/**
 * AJAX Callback function to return a list of media files 
 */
function media_browser_list_experimental() {
  $params = drupal_get_query_parameters();
  // How do we validate these?  I don't know.
  // I think PDO should protect them, but I'm not 100% certain.
  array_walk_recursive($params, '_media_recursive_check_plain');
  
  $types = isset($params['types']) ? $params['types'] : NULL;
  $url_include_patterns = isset($params['url_include_patterns']) ? $params['url_include_patterns'] : NULL;
  $url_exclude_patterns = isset($params['url_exclude_patterns']) ? $params['url_exclude_patterns'] : NULL;
  

  $start = isset($params['start']) ? $params['start'] : NULL;
  $limit = isset($params['limit']) ? $params['limit'] : NULL;
  
  $conditions = array();

  if ($types) {
    $conditions[] = array('type', $types, 'IN');
  }

  if ($url_include_patterns) {
    $conditions[] = array('uri', "%$v%", 'like');
    // Insert stream related restrictions here.
  }
  if ($url_exclude_patterns) {
    $conditions[] = array('uri', "%$v%", 'not like');
    // Insert stream related restrictions here.
  }
  
  $entity_controller = entity_get_controller('media');
  $media_entities = $entity_controller->load(NULL, $conditions, $start, $limit);

  foreach ($media_entities as &$media) {
    $media->preview = drupal_render(_media_format_for_browser($media));
  }
  
  drupal_json_output(array('media' => array_values($media_entities)));
  die();

}

function _media_format_for_browser($media) {
  $media->override = array('browser' => TRUE);
  // Generate a preview of the file
  // @todo: Should generate placeholders for audio
  // Add yet another wrapper so we can style it as a preview :(
  // Otherwise it isn't really possible to know because the user can pick anything for their preview mode.
  $preview = field_view_field('media', $media, 'file', 'media_preview');
  $preview['#show_names'] = TRUE;
  $preview['#theme_wrappers'][] = 'media_thumbnail';
  return $preview;
}

function media_media_browser_new_plugins($params) {
  $plugins = array();
  $path = drupal_get_path('module', 'media');

  $types = isset($params['types']) ? $params['types'] : array();
  include_once($path . '/media.admin.inc');
  include_once($path . '/media.pages.inc');

  $redirect = array('media/browser', array('query' => array('render' => 'media-popup')));

  $upload_form = drupal_get_form('media_add_upload', $redirect);
  $from_url_form = drupal_get_form('media_add_from_url', $redirect);
  // These changes to #type = tabpage probably
  // Add the Upload tab.
  $plugins['upload'] = array(
    '#title' => t('Upload'),
    '#markup' => drupal_render($upload_form),
    '#attached' => array(
      //'js' => array($path . '/javascript/plugins/media.upload.js'),
    ),
  );

  // Add the 'From URL' tab.
  $plugins['fromurl'] = array(
    '#title' => t('From URL'),
    '#markup' => drupal_render($from_url_form),
    '#attached' => array(
      //'js' => array($path . '/javascript/plugins/media.fromurl.js'),
    ),
  );

  // Add the default 'Library' tab.
  $plugins['library'] = array(
    '#title' => t('Library'),
    '#attached' => array(
      'js' => array(
        $path . '/javascript/plugins/media.library.experimental.js',
      ),
      'css' => array(
        $path . '/javascript/plugins/media.library.css',
      ),
    ),
    '#settings' => array(
      'viewMode' => 'thumbnails',
      'getMediaUrl' => url('media/browser/experimental/list'),
      'types' => $types,
    // We should probably change this to load dynamically when requested
    // via the JS file.
    ),
    '#markup' => '<div id="container"><div id="scrollbox"><ul id="media-browser-library-list"></ul></div><div id="status"></div></div>',
  );

  return $plugins;
}

/**
 * @todo: Re-integrate the stream based filters, etc
 * Currently this function is not being used.  See media_browser_experimental_list
 *
 *  Any options needed to retrieve the file select options. An associate array
 *  with the following optional keys:
 *    'conditions' => An array of 'column' => 'value' to pass to the db query,
 *    'streams' => An array of streams to filter,
 *    'limit' => The number of results to return.
 */
function media_browser_fid_list($options = array()) {
  $conditions = isset($options['conditions']) ? $options['conditions'] : array();
  $streams = isset($options['streams']) ? $options['streams'] : array();
  $limit = isset($options['limit']) ? $options['limit'] : media_variable_get('browser_pager_limit');


  $entity_controller = entity_get_controller('media');
  
  $results = $entity_controller->load(NULL, $conditions);
  return $results;
  // @TODO: re-integrate this stuff.

  // First get the fid's to load. We have to do that first, because
  // entity_load doesn't accept a condition of LIKE, which we need for streams.
  $select = db_select('file', 'f')
    ->extend('PagerDefault')
    ->fields('f', array('fid'));

  // Filter on streams.
  foreach ($streams as $stream) {
    $select->condition('uri', db_like($stream) . '%', 'LIKE');
  }

  // Add our conditions.
  foreach ($conditions as $field => $condition) {
    $select->condition($field, $condition);
  }

  // Add our pager limit filter.
  $select->limit($limit);

  // Grab the uri's.
  $fids = $select->execute()
    ->fetchCol();

  $selections = array();

   if (!empty($fids)) {
    // Now load the desired media to display.
    $medias = entity_load('media', $fids);

    
  }

  return $selections;
}

/**
 * Silly function to recursively run check_plain on an array.
 *
 * There is probably something in core I am not aware of that does this.
 * 
 * @param $value
 * @param $key
 */
function _media_recursive_check_plain(&$value, $key) {
  $value = check_plain($value);
}

// Hook Docs: Will eventually go in API file.

/**
 * Plugins are expected to return a keyed array of renderable elements.
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
 *       $path . '/javascript/plugins/media.library.experimental.js',
 *     ),
 *   ),
 *   '#settings' => array(
 *     'viewMode' => 'thumbnails',
 *     'getMediaUrl' => url('media/browser/experimental/list'),
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
function hook_media_browser_new_plugins($params) {

}