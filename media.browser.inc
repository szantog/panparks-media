<?php
// $Id$

/**
 *  @file
 *  The media popup browser for editors.
 */

/**
 *  Displays the popup media browser for editors.
 */
function media_browser() {
  if (media_variable_get('experimental_browser')) {
    module_load_include('inc', 'media', 'media.browser.experimental');
    return media_browser_experimental();
  }

  $rows = array();

  $attached = array();
  $attached['library'][] = array('media', 'media_browser_page');


  if (isset($_GET['fid']) && is_numeric($_GET['fid'])) {
    // Temporary hack.  God this is too many cooks in a messy kitchen.
    // Basically, this just returns here, and there is other crazy code OUTSIDE
    // of the browser to determine that an upload has happened, and we should exit
    // the browser, selecting that one.
    // see media.popups.js.
    // This code seriosuly needs to change, but the code that was here before
    // was heinously broken and assumed a format form should show after every upload.
    // This does not make sense when using media as a field.

    $media = entity_load('media', array($_GET['fid']));
    if (!empty($media)) {
      $media = array_pop($media);
      if ($media->fid) {
        drupal_add_js(array(
          'media' => array(
            'selectedMedia' => array($media),
            )
          )
        );
      }
    }

    $plugins = array();
  }
  
  else {
    // If the media is already selected, we don't need to load plugins.
    // This is a god-awful way to do this, but it is there, so just trying
    // to support it till it can be replaced.

    $plugins = module_invoke_all('media_browser_plugins');

    // What can plugins do?
    // Plugins can declare a JS class which they use to add stuff.. Okay.
    // Plugins can add tabs?

    // Alternate model
    // There are no plugins, just a big form alter.
    // Cons:
    // API control is lost.  This makes upgrades near impossible, testing too.

    // Alternate model
    // Hybrid: Plugins can add tabs, and they can add options for those tabs
    // Q: Can the tabs submit any forms?
    // Q: What do the tabs contain?  Either HTML or a URL
    // Q: What if a plugin wants to modify an existing tab?  (that tab should provide a UI).

  }

  $plugins = module_invoke_all('media_browser_plugins');
  
  // What can plugins do?
  // Plugins can declare a JS class which they use to add stuff.. Okay.
  // Plugins can add tabs?

  // Alternate model
  // There are no plugins, just a big form alter.
  // Cons:
  // API control is lost.  This makes upgrades near impossible, testing too.

  // Alternate model
  // Hybrid: Plugins can add tabs, and they can add options for those tabs
  // Q: Can the tabs submit any forms?
  // Q: What do the tabs contain?  Either HTML or a URL
  // Q: What if a plugin wants to modify an existing tab?  (that tab should provide a UI).
  
  foreach ($plugins as $name => $plugin) {
    // Add their JS and CSS
    if ($plugin['#attached']) {
      $attached = array_merge_recursive($attached, $plugin['#attached']);
    }
    unset($plugin['#attached']);
  }

  // Allow modules implementing hook_media_browser_callbacks() to add a set
  // of callbacks to the settings.
  // @TODO: Document the hook...
  $callbacks = module_invoke_all('media_browser_callbacks');
  drupal_alter('media_browser_callbacks_alter', $callbacks);

  // Allow modules implementing hook_media_conditions to add new
  // conditions to load the library. For instance, they may only return media
  // belonging to the user, with array('uid' => $user->uid);
  // @TODO: Not sure if we want this here specifically; we have an alter.
  $conditions = module_invoke_all('media_conditions');

  // Allow modules implementing hook_media_streams to filter streams
  // when loading the library. For instance, they may only return media
  // belonging to the flickr:// stream, with array('flickr://');
  // @TODO: Not sure if we want this here specifically; we have an alter.
  $streams = module_invoke_all('media_streams');

  // Pass our settings to the browser.
  $settings = array(
    'viewType' => media_variable_get('browser_viewtype_default'),
    'callbacks' => $callbacks,
    'plugins' => $plugins,
    'conditions' => $conditions,
    'streams' => $streams,
  );
  drupal_add_js(array('media' => array('browser' => $settings)), 'setting');

  // Here we will return the basic structure of the browser.
  $build['media_browser'] = array(
    '#prefix' => '<div id="media-browser">',
    '#suffix' => '</div>',
    '#attached' => $attached,
  );

  // Display the browser tabs.
  $build['media_browser']['tabs'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="media-browser-tabs">',
    '#suffix' => '</div>',
    '#markup' => '<ul></ul>',
  );

  return $build;
}

/**
 * Implement hook_media_browser_plugins
 * @return unknown_type
 *
 * @see media_add_upload()
 * @see media_add_from_url()
 */
function media_media_browser_plugins() {
  $plugins = array();
  $path = drupal_get_path('module', 'media');
  include_once($path . '/media.pages.inc');

  $redirect = array('media/browser', array('query' => array('render' => 'media-popup')));

  $upload_form = drupal_get_form('media_add_upload', $redirect);
  $from_url_form = drupal_get_form('media_add_from_url', $redirect);

  // Add the Upload tab.
  $plugins['upload'] = array(
    '#attached' => array(
      'js' => array($path . '/javascript/plugins/media.upload.js'),
    ),
    'settings' => array(
      'uploadForm' => drupal_render($upload_form),
    ),
  );

  // Add the 'From URL' tab.
  $plugins['fromurl'] = array(
    '#attached' => array(
      'js' => array($path . '/javascript/plugins/media.fromurl.js'),
    ),
    'settings' => array(
      'fromUrlForm' => drupal_render($from_url_form),
    ),
  );

  // Add the default 'Library' tab.
  $plugins['library'] = array(
    '#attached' => array(
      'js' => array(
        $path . '/javascript/plugins/media.library.js',
      ),
      'css' => array(
        $path . '/javascript/plugins/media.library.css',
      ),
    ),
    'settings' => array('viewMode' => 'thumbnails'),
  );

  return $plugins;
}

/**
 * Implements hook_media_browser_callbacks().
 */
function media_media_browser_callbacks() {
  // The callback for the default media library thumbnail list.
  return array (
    'getMedia' => array(
      'url' => url('media/browser/list'),
      'token' => drupal_get_token('media/browser/list'),
    ),
  );
}

/**
 * AJAX Callback for a list of media with some basic filters.
 *
 * @return unknown_type
 */
function media_browser_list() {
  if (isset($_GET['conditions'])) {
    $conditions = drupal_json_decode($_GET['conditions']);
  }
  $conditions = isset($conditions) ? $conditions : array();

  // If we have been passed an array of streams, such as 'youtube://',
  // then only accept media from those streams. Unfortunately, entity_load
  // doesn't accept a filter of LIKE, so we need to preprocess that.
  // @TODO: However, if we pass more than one stream, it will do AND rather
  // than OR right now (hopefully an easy fix).
  if (isset($_GET['streams'])) {
    $streams = drupal_json_decode($_GET['streams']);
  }
  $streams = isset($streams) ? $streams : array();

  // Allow modules implementing hook_media_conditions_alter to alter
  // the set of browser library conditions.
  drupal_alter('media_conditions', $conditions);

  // Allow modules implementing hook_media_streams_alter to alter
  // the set of browser stream filters.
  drupal_alter('media_streams', $streams);

  // @TODO: This will get rather large I suspect, unless we combine
  // with pagers, or use a db_transaction maybe, or just write our own query.

  // First get the fid's to load. We have to do that first, because
  // entity_load doesn't accept a condition of LIKE, which we need for streams.
  $select = db_select('file', 'f')//->extend('PagerDefault')
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
//   $select->limit(media_variable_get('browser_pager_limit'));

  // Grab the fid's.
  $fids = $select->execute()
    ->fetchCol();

  if (!empty($fids)) {
    $medias = entity_get_controller('media')->load($fids);
    foreach ($medias as &$media) {
      $media->override = array('browser' => TRUE);
      // Generate a preview of the file
      // @todo: Should generate placeholders for audio
      // Add yet another wrapper so we can style it as a preview :(
      // Otherwise it isn't really possible to know because the user can pick anything for their preview mode.
      $preview = field_view_field('media', $media, 'file', 'media_preview');
      $preview['#show_names'] = TRUE;
      $preview['#theme_wrappers'][] = 'media_thumbnail';

      $media->preview = drupal_render($preview);
    }
    drupal_json_output(array('media' => $medias/*, 'pager' => theme('pager')*/));
    drupal_exit();
  }
  else {
    // Print an empty message.
    drupal_json_output(array('empty' => t(media_variable_get('browser_library_empty_message')), 'media' => array()));
    drupal_exit();
  }
}

/**
 * Prepares the page to be able to launch the media browser.
 *
 * @TODO: This is a WTF at present.  Basically, the browser is launched from wysiwyg
 * and from fields, so having a common function makes sense.  But not like this.
 *
 * Defines default variables.
 */
function media_include_browser_js() {
  static $included;
  if ($included) {
    return;
  }
  $included = TRUE;
  drupal_add_library('media', 'media_browser');

  $settings = array(
    'browserUrl' => url('media/browser',
      array('query' => array('render' => 'media-popup'))),
    'styleSelectorUrl' => url('media/-media_id-/format-form',
      array('query' => array('render' => 'media-popup'))),

    // Adding src to blacklist; fid and view_mode we capture outside attribs so adding
    // them too to the blacklist.
    'blacklist' => array('src','fid','view_mode'), // Only applies to WYSIWG - should be removed;
  );

  drupal_add_js(array('media' => $settings), 'setting');
}


/**
 * Menu callback for testing the media browser
 */
function media_browser_testbed($form) {
  media_include_browser_js();
  $launcher = '<a href="#" id="launcher"> Launch Media Browser</a>';

  $form['options'] = array(
    '#type' => 'textarea',
    '#title' => 'Options (JSON)',
    '#rows' => 10,
  );

  $form['launcher'] = array(
    '#markup' => $launcher,
  );

  $form['result'] = array(
    '#type' => 'textarea',
    '#title' => 'Result',
  );
  

  $js = <<<EOF
    Drupal.behaviors.mediaTest = {
    attach: function(context, settings) {
      var delim = "---";
      var recentOptions = [];
      var recentOptionsCookie = jQuery.cookie("recentOptions");
      if (recentOptionsCookie) {
        recentOptions = recentOptionsCookie.split("---");
      }

      var recentSelectBox = jQuery('<select id="recent_options" style="width:100%"></select>').change(function() { jQuery('#edit-options').val(jQuery(this).val())});

      jQuery('.form-item-options').append('<label for="recent_options">Recent</a>');
      jQuery('.form-item-options').append(recentSelectBox);
      jQuery('.form-item-options').append(jQuery('<a href="#">Reset</a>').click(function() {alert('reset'); jQuery.cookie("recentOptions", null); window.location.reload(); }));
      
      jQuery.each(recentOptions, function (idx, val) {
        recentSelectBox.append(jQuery('<option></option>').val(val).html(val));
      });


      jQuery('#launcher').click(function () {
        jQuery('#edit-result').val('');
        var options = {};
        var optionsTxt = jQuery('#edit-options').val();
        if (optionsTxt) {
          // Store it in the recent box
          recentOptionsCookie += "---" + optionsTxt
          jQuery.cookie("recentOptions", recentOptionsCookie, { expires: 7 });
          recentSelectBox.append(jQuery('<option></option>').val(optionsTxt).html(optionsTxt));
          options = eval('(' + optionsTxt + ')');
        }
        Drupal.media.popups.mediaBrowser(Drupal.behaviors.mediaTest.mediaSelected, options);
        return false;
      });
    },
    
    mediaSelected: function(selectedMedia) {
      var result = JSON.stringify(selectedMedia);
        jQuery('#edit-result').val(result);
    }
  }

EOF;
  
  drupal_add_js($js, array('type' => 'inline'));
  return $form;
  
}