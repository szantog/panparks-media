
(function ($) {

/**
 * Quick hacked up media browser for testing communication
 */
Drupal.behaviors.mediaBrowser = {
  attach: function(context, settings) {
    // The problem is, we don't want to start this when it loads
    // Because, the parent (if in an iframe)
    // Wants to pass options and tell it to start :(
    // We should be starting here IF the parent doesn't
    // initiate the process.
    // For testing  
    if (window.location.href.search('autostart') != -1) {
      Drupal.media.browser.launch({});
      $('#media-browser-page-wrapper')
        .width('800px')
        .height('500px')
        .css('border', '1px solid black')
        .css('overflow', 'hidden');
    };
    //var startLink = $('<a href="#">Start browser</a>');
    //$('h1').append(startLink);
    //startLink.bind('click', function() {Drupal.media.browser.launch()});

    // This method is called when the popup appears.  Use this opportunity to
    // construct the tabs.
    // TODO: Hook it up so that visiting the library tab calls the start method.
    
  }
};

namespace('Drupal.media.browser');

Drupal.media.browser.selectedMedia = [];
Drupal.media.browser.plugins = {};

Drupal.media.browser.getDefaults = function () {
  return {
    viewType: 'thumbnails',
    //@todo: use variable
    callbacks: Drupal.settings.media.browser.callbacks,
    plugin_settings: Drupal.settings.media.browser.plugins,
    defaultTab: Drupal.settings.media.browser.defaultTab,
    conditions: {}
  }
};
  
Drupal.media.browser.launch = function(options) {
  this.settings = $.extend({}, this.getDefaults(), options);
  this.setupListeners();
  this.setupTabs();
  this.startAll();
  // This is retarded.  Can't figure out how to make it call onload
  // when on tab 0, so we'll just switch to 2 :)
  this.getTabset().tabs('select', 2);
  //this.getTabset().tabs('select', this.settings.defaultTab);
};

// Most likely this should be in a plugin (like Core plugin or something).
Drupal.media.browser.setupListeners = function() {
  this.listen('mediaSelected', this.mediaSelected);
}

Drupal.media.browser.mediaSelected = function(e, data) {
  Drupal.media.browser.selectedMedia = data.mediaFiles;
}

Drupal.media.browser.setupTabs = function() {
  var that = this;
  this.getTabset().tabs({
    select: function (e, ui) {
      // This is kinda silly, but I don't know how else to send the identifier upstream
      that.notify('tabs.selected', jQuery(ui.tab).attr('href').slice('1'));
    },
    show: function (e, ui) {
      // This is kinda silly, but I don't know how else to send the identifier upstream
      that.notify('tabs.show', jQuery(ui.tab).attr('href').slice('1'));
    }
  });
}


Drupal.media.browser.register = function (pluginId, factory, options) {
  this.plugins[pluginId] = {
    factory: factory,
    instance: null,
    options: options // Currently this is not used.  @todo, figure this out.
  };
};

Drupal.media.browser.start = function (pluginId) {
  // Fire their init events, pass in this
  // @todo: decouple and provide an interface.
  debug.debug('Starting Plugin: ' + pluginId);
  //This isn't ideal, the settings thing needs more definition, anyway,
  // allows us to merge in settings that could have been modified.
  // 
  debug.debug(this.settings);
  if (typeof this.settings.plugin_settings[pluginId] != 'undefined') {
    if (typeof this.settings.plugin_settings[pluginId] != 'undefined') {
      var options = this.settings.plugin_settings[pluginId].settings;
    }
  }
  
  debug.debug(options);
  this.plugins[pluginId].instance =
    this.plugins[pluginId].factory(this, options);
  this.plugins[pluginId].instance.init();
};

Drupal.media.browser.stop = function (pluginId) {
  var data = this.plugins[pluginId];
  if (data.instance) {
    data.instance.destroy();
    data.instance = null;
  };
};

Drupal.media.browser.startAll = function() {
  // Loop through the provided plugins
  // from Drupal.settings.media.plugins (or something)
  for (var pluginId in this.plugins) {
    debug.debug("Starting plugin " + pluginId);
    this.start(pluginId);
  };
};

Drupal.media.browser.stopAll = function() {
  for (var pluginId in this.plugins) {
    this.stop(pluginId);
  };
};

Drupal.media.browser.getCallbackUrl = function(name) {
  //@todo: error handling here.
  return this.settings.callbacks[name].url;
}

/**
 * Interfaces to be used by plugins
 */

Drupal.media.browser.listen = function (eventName, callback) {
  jQuery(this.getRootElement()).bind(eventName, callback);
};

Drupal.media.browser.notify = function (eventName, data) {
  jQuery(this.getRootElement()).trigger(eventName, data);
};

Drupal.media.browser.getRootElement = function() {
  return $('#media-browser');
}

Drupal.media.browser.getTabset = function() {
  return $('#media-browser-tabs');
}

/**
 * Returns the active tab panel
 */
Drupal.media.browser.getActivePanel = function() {
  var selectedTabIndex = this.getTabset().tabs('option', 'selected');
  if (selectedTabIndex == -1) {
    // If nothing is selected return an empty element (I guess...).
    return $();
  }
  debug.debug(selectedTabIndex);
  debug.debug($('#media-browser-tabs .ui-tabs-panel:eq(' + selectedTabIndex + ')'));
  return $('#media-browser-tabs .ui-tabs-panel:eq(' + selectedTabIndex + ')')
}

})(jQuery);