<?php
// $Id$

/**
 * @file
 * Experimental browser, testing alternative FAPI integration with the popup.
 *
 * To use this, you'll need to add the following variable override to your
 * settings.php:
 *   $conf['media__experimental_browser'] = TRUE;
 */

function media_browser_experimental($selected = NULL) {
  $output = array();
  $output['#attached']['library'][] = array('media', 'media_browser_page_experimental');

  if (isset($_GET['fid'])) {
    // A form has submitted, selected a piece (or pieces) of media and we refreshd.
    // So we're just going to send the settings out.
    if (!is_array($_GET['fid'])) {
      $selected = array($_GET['fid']);
    }
    $selectedMedia = entity_load('media', $selected);
    $setting = array('media' => array('selectedMedia' => array_values($selectedMedia)));
    drupal_add_js($setting, 'setting');
    return $output;
  }
  

  // Any values which need to go to the media browser will come here.
  // How do we validate these?  I don't know.
  // I think PDO should protect them, but I'm not 100% certain.
  $params = isset($_GET['params']) ? $_GET['params'] : array();
  array_walk_recursive($params, '_media_recursive_check_plain');
  $plugins = module_invoke_all('media_browser_new_plugins', $params);
  
  $list = array();

  $settings = array('media' => array('browser' => array()));
  $browser_settings =& $settings['media']['browser'];

  //@todo: replace with Tabs module if it gets upgraded.
  foreach (element_children($plugins) as $key) {
    $plugin =& $plugins[$key];
    //Add any JS settings
    $browser_settings[$key] = array();
    if (isset($plugin['#settings'])) {
      $browser_settings[$key] += $plugin['#settings'];
    }
    $href = isset($plugin['#callback']) ? $plugin['#callback'] : "#media-tab-$key";
    $list[] = "<a href='$href'><span>{$plugin['#title']}</span></a>";
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
    '#markup' => '<ul><li>' . implode('</li><li>', $list) . '</li></ul>'
  );
  
  $output['tabset']['plugins'] = $plugins;
  return $output;
}


function media_browser_list_experimental() {
  $params = $_GET;
  array_walk_recursive($params, '_media_recursive_check_plain');

  if (isset($params['types'])) {
    $types = $params['types'];
  }

  $start = isset($params['start']) ? $params['start'] : NULL;
  $limit = isset($params['limit']) ? $params['limit'] : NULL;
  
  $conditions = array();

  if ($types) {
    $conditions[] = array('type', $types, 'IN');
  }

  if ($url_include_patterns) {
    $conditions[] = array('uri', "%$v%", 'like');
    // Insert stream related restrictions here like:
  }

  //$streams = isset($options['streams']) ? $options['streams'] : array();
  //$limit = isset($options['limit']) ? $options['limit'] : media_variable_get('browser_pager_limit');

  $entity_controller = entity_get_controller('media');
  $media_entities = $entity_controller->load(NULL, $conditions, $start, $limit);

  foreach ($media_entities as &$media) {
    $media->preview = drupal_render(_media_format_for_browser($media));
  }

  print drupal_json_encode(array('media' => array_values($media_entities)));
  die();

}

/**
 * Build a form listing available media files.
 * @todo: decide if this will even work...
 * Problems: Don't want to build unless the user goes to that tab
 * Also, how do you add additional checkboxes later?
 * Or change params?
 *
 * @param array $form
 *  The form built with FAPI.
 * @param array &$form_state
 *  The current form state after rendering.
 * @param array $types
 *  An array media types to show.
 * @param array $url_include_patterns
 *  An array of url patterns to show (OR'd).
 * @param array $url_exclude_patterns
 *  An array of url patterns to hide.
 */
function media_browser_library($form, &$form_state, $types = array(), $url_include_patterns = array(), $url_exclude_patterns = array()) {
  $options = array();
  $options['conditions'] = array();

  if ($types) {
    $options['conditions'][] = array('type', $types, 'IN');
  }
  
  if ($url_include_patterns) {
    // Insert stream related restrictions here like:
    foreach($url_include_patterns as $v) {
      $options['conditions'][] = array('uri', "%$v%", 'like');
    }
  }
  $media_entities = media_browser_fid_list($options);

  // Now build the "form"

  $form['media'] = array();

  foreach ($media_entities as $media) {
    $form['media']['media_' . $media->fid] = array(
      '#type' => 'checkbox',
      '#preview' => _media_format_for_browser($media),
      // Not sure I like putting a bogus attribute here...
      '#attributes' => array('fid' => $media->fid),
    );
  }

  return $form;
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

  if (isset($params['types'])) {
    // This could be use to add validators to the upload form
    $types = $params['types'];
  }
  
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
    // We should probably change this to load dynamically when requested
    // via the JS file.
    ),
    '#markup' => '<div id="container"><div id="scrollbox"><ul id="media-browser-library-list"></ul></div><div id="status"></div></div>',
  );

  return $plugins;
}

/**
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

function _media_recursive_check_plain(&$value, $key) {
  $value = check_plain($value);
}
