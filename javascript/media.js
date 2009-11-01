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



/* **************************************************** */
/* Content browser navigation                           */
/* **************************************************** */

/**
 * @NOTE arthur added this stuff in to prep for the new UI.
 * This connects navigation elements (results limters, pagers)
 * to an ajax function that will reload the content navigator
 * content.
 */

$(document).ready(function () {

  /** 
   * Load the Media Browser for the first time
   */
  $('.media_browser_activation a').bind('click', function() {
    // We need to check if the dialog box was already instantiated. If
    // it has been, the dialog box will have UI classes
    if ($('#dialog').hasClass('ui-dialog-content')) {   
      // Remove any content from the browser pane
      $('#media_content_browser').html();
      $('#dialog').dialog('open');
    }
    // Create our dialog box.
    else {    
      $('#dialog').dialog({
        buttons: { "Ok": function() { $(this).dialog("close"); } }, 
        modal: true,
        draggable: false,
        resizable: false,
        minWidth: 600,
        width: 800,
        position: 'center',
        overlay: {
          backgroundColor: '#000000',
          opacity: 0.3
        }
      });
    }
    // Get the current query string
    var query = $(this).attr('href').replace(/.*\?/, '');    
    // Load our content from ajax and style 
    media_load_content_display(query);
    return false;
  });
   
   
  /**
   *  Catch the clicks on the result limiters and pager queries
   *  and modify the links to have the options in their URL so   
   *  they can be parsed. Note that we are using the live function
   *  so that updates to the media_content_browser still have
   *  jquery functionality
   */  
  $('ul.result_limit li a, ul.pager li a').live('click', function() {
    // Get the current query string
    var query = $(this).attr('href').replace(/.*\?/, '');
    media_load_content_display(query);
    // Make sure to stop the click
    return false;
  });
  
  
  function media_load_content_display(query) {
    // Start visual effects
    // Show the throbber
    $('#media_content_browser_throbber').fadeIn('fast');    
    $('#media_content_browser').fadeTo('slow', 0.23, function() {      
      // Fetch content data with the new parameters    
      media_load_content_navigator_reload(query);      
      // Hide throbber
      $('#media_content_browser_throbber').fadeOut('slow', function () {
        // fade back the content
        $('#media_content_browser').fadeTo('slow', 1);
        });      
        
    }); // fade    
  }
  
  /**
   * Stub function to get the current active pager
   * and reload the content based on this navigators
   * query string
   */
  function media_load_content_navigator_reload(query) {
    $.getJSON(Drupal.settings.media.media_browser_content_load_url+'?'+query,     
      function(data) { $('#media_content_browser').html(data);}
    );
  }
  
  
  /**
   * Handle the clicks on actual images and transfer those values 
   * to the submission form
   */
   // Catch the clicks on the images in the modal window
   $('.media-thumbnail ul.media_content_navigator.results a').live('click', function () {
     // Remove any current selections
     $('.media-thumbnail').removeClass('selected');     
     // Select this thumbnail
     $(this).parents('.media-thumbnail').addClass('selected');     
     // We need to get the value of the checkbox for this selected file
     var uri = $('.media-thumbnail.selected input.form-checkbox').val();
     // Get the enclosing div and then the file input field and change its value
     // @TODO this does not support multiple file fields currently because
     //       the modal window appends to the bottom of the page. We need
     //       to come up with a mechanism to snif the current field- placing
     //       a hiden element into the modal dailog with the filefield id 
     //       might work well
     $('input.media-file-uri').val(uri);   
     // Deactivate the click
     return false;
     });
  
}); // $(document).ready


})(jQuery);