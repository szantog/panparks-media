// $Id$

/**
 *  @file
 *  This file handles the JS for Media Module functions.
 */


/**
 *  This handles the display, activation
 *  and hiding of drawers on the media browser form.
 */
(function ($) {
Drupal.behaviors.mediaDrawers = {
  attach: function (context, settings) {

    // Hide all the drawer display items on page load.
    $('.media.browser .ui-tabs-panel .drawer.display', context).once('mediaDrawersHide', function() {
      $(this).hide();
    });

    // Activate all the drawer data display that needs to be shown when
    // the page is loaded.
    $('.media.browser .ui-tabs-panel .drawer.display.active', context).once('mediaDrawersActivate', function() {
      $(this).show();
    });

    // --------------------------------------
    // Click actions.

    // Now we need to bind click functionality on drawers to display.
    $('.media.browser .drawers .item-list ul li, .drawers li a', context).once('mediaDrawersBind').bind('click', function () {
      // Get the href id that we want to display.
      var display_id = $(this).attr('href');
      // This handles the LI click.
      if (display_id == undefined) {
        var display_id = $(this).children('a').attr('href');
      }
      // We need to get the tab page that this drawer is in.
      var parent = $(this).parents('.ui-tabs-panel').attr('id');
      // Hide current active drawer display.
      $('#'+parent+' .drawer.display.active').removeClass('active').hide();
      // Set any drawers to not active.
      $('#'+parent+' .drawers li.active').removeClass('active');
      // Make this drawer active.
      $(this).addClass('active');
      // Make the requested drawer display active.
      $(display_id).addClass('active').show();
    });
  }
}
/**
 *  We need to hide any form elements that were replaced by the media browser
 *  form, activate the add button, and hide the browser.
 */
Drupal.behaviors.mediaBrowserHide = {
  attach: function (context, settings) {
    // Hide our file progress indicators.
    $('.media-browser-file-progress', context).once('mediaBrowserHide').attr('style', 'display:none');
    $('.media-browser-metadata-wrapper', context).once('mediaBroswerHide').hide();
    $('.media-browser-drawer-select', context).once('mediaBroswerHide').attr('style', 'display:none');
    $('.media-browser-metadata-submit', context).once('mediaBroswerHide', function() {
      $(this).attr('style', 'display:none').click(function() {
        $(this).hide().siblings('.media-browser-metadata-wrapper').slideUp();
      });
    });

    // Add behavior to our big red activation button.
    $('.media.browser.activation', context).once('mediaBrowserHide', function () {
      // Hide the browser associated with this button.
      $(this).next('.media.browser').hide();
      $(this).click(function () {
        // When clicking, show the browser and hide this button.
        $(this).next('.media.browser').slideDown('slow');
        $(this).slideUp();
      });
    });
  }
}
/**
 *  Generate a MD5 hash of the file being uploaded.
 */
Drupal.behaviors.mediaGenerateMD5 = {
  attach: function (context, settings) {
    // Get the value from the file field.
    $('#edit-field-file-media-media-tabs-tab-Addfiles-media-upload-resource-Newfile-resource-form-media-upload-upload', context).once('mediaGenerateMD5').change(function () {
      // Add the MD5 hash from the file name to the upload URL.
      Drupal.settings['ahah']['edit-attach']['url'] += '/'+$.md5($(this).val());
      // @TODO: Now add the MD5 value to the meta data form.
      alert(Drupal.settings['ahah']['edit-attach']['url']);
    });
  }
}

/**
 *  Hide the browser and bind click behavior.
 */
Drupal.behaviors.mediaAhahHideBrowser ={
  attach: function (context, settings) {
    $('.media-browser-submit', context).once('mediaAhahHideBrowser', function() {
      $(this).bind('click', function() {
        $(this).hide().siblings('.ui-tabs-panel, ul').slideUp();
        $(this).siblings('.media-browser-file-progress').show();
        $(this).siblings('.media-browser-metadata-wrapper').show();
        $(this).siblings('.media-browser-metadata-submit').show();
      });
    });
  }
}
})(jQuery);