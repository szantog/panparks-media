// $Id$

/**
 *  @file
 *  This file handles the JS for Media Module functions.
 */

/*

The naming convetion for items in this file revolves around three different
components: tabs, subtabs and panes. Tabs are a primary navigation or grouping
mechanism. Subtabs are related to a specific tab. Panes are the content
related to the subtab.

   ---------------------------------------
   |                  |tab | tab2 | tab3 |
   |                  --------------------
   |__________ __________________________|
   | Subtab  |                           |
   -----------                           |
   | Subtab2 |        Pane for           |
   -----------      active subtab        |
   | Subtab3 |                           |
   -----------                           |
   |                                     |
   |                                     |
   |                                     |
   ---------------------------------------



*/

(function ($) {


/* **************************************************** */
/* Content browser navigation                           */
/* **************************************************** */


/**
 * Loads media browsers and callbacks, specifically for media as a field.
 */
  
Drupal.behaviors.mediaBrowserFields ={
  attach: function (context, settings) {
    $('.form-media-file', context).once('mediaBrowserLaunch', function () {
      
      var fidField = $('.fid', this);
      fidField.show();
      var previewField = $('.preview', this);
      if (Drupal.settings.media.debug) {
        var debugField = $('.file_info', this);
      }
      $('.launcher', this).bind('click', function () {
        alert('Needs to be fixed to use the new media.popups.js, see wysiwyg integration in media.filter.inc');
        options = {};
        $().mediaBrowser(
          function(mediaFiles) {
            if (mediaFiles.length < 0) {
              return;
            }
            mediaFile = mediaFiles[0];
            fidField.val(mediaFile.fid);
            previewField.html(mediaFile.preview);
            if (Drupal.settings.media.debug) {
              debugField.html(JSON.stringify(mediaFile));
            }
          },
          options
        );
        // open the iframe, and implement the onLoad
        //@todo make the url a variable.
        return false;
      });
    });
  }
};

/**
 * @NOTE arthur added this stuff in to prep for the new UI.
 * This connects navigation elements (results limters, pagers)
 * to an ajax function that will reload the content navigator
 * content.
 */

$(document).ready(function () {




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
    $('div.pane_content.active').fadeTo('slow', 0.23, function() {
      // Fetch content data with the new parameters
      media_load_content_navigator_reload(query);
      // Hide throbber
      $('#media_content_browser_throbber').fadeOut('slow', function () {
        // fade back the content
        $('div.pane_content.active').fadeTo('slow', 1);
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
      function(data) { $('div.pane_content.active').html(data);}
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