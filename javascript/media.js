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
 * Load the Media Browser for the first time
 */
Drupal.behaviors.mediaBrowserLaunch ={
  attach: function (context, settings) {
    $('input.media-file-uri ', context).once('mediaBrowserLaunch', function () {
      $(this).bind('click', function () {
        // Add the dialog box to the page using this file field id
        $(this).append(media_render_dialog());

        // Now we set the current data state for the media browser by storing
        // the data in a hidden field
        $('#media_browser_data_store').attr('tabname', $('#media_content_browser_tabs .item-list a.active').attr('tabname')).attr('object-type', $(this).attr('object-type')).attr('bundle', $(this).attr('bundle')).attr('field-name', $(this).attr('field-name'));

        // Render the subtabs for the current active tab
        renderSubTabs();

        // Now we can launch our modal window
        $('#dialog').dialog({
          buttons: { "Ok": function() { $(this).dialog("close"); } },
          modal: true,
          draggable: false,
          resizable: false,
          minWidth: 600,
          width: 800,
          position: 'top',
          overlay: {
            backgroundColor: '#000000',
            opacity: 0.4
          }
        });

        return false;
      });
    });
  }
};


/**
 * Render the modal dialog box
 */
function media_render_dialog() {
  // If the dialog box already exists, we need to remove it from the DOM
  if ($('#dialog.ui-dialog-content')) {
    $('#dialog').dialog('destroy');
    $('#dialog').remove('#dialog');
  }
  // Build out the full HTML structure for the dialog box
  var html = '<div id="dialog" style="display: none;" title="Select your files"> \
    <div id="media_content_browser_throbber"></div> \
    <div id="media_content_browser_tabs">'+Drupal.settings.media.media_browser_tabs+'</div> \
    <div id="media_content_browser_subtabs"></div> \
    <input type="hidden" id="media_browser_data_store" /> \
  </div>';
  return html;
}


/**
 * Attach the behaviors to the main tabs across the top
 * of the browser
 */
Drupal.behaviors.tabsClick ={
  attach: function (context, settings) {
    // Enable our tabs
    $('#media_content_browser_tabs .item-list a', context).once('tabsClick', function () {
      $(this).bind('click', function () {
        // The first thing we need to do is to update the data store with this tab data
        $('#media_browser_data_store').attr('tabname', $(this).attr('tabname'));

        // Unset other active tabs
        $('#media_content_browser_tabs .item-list li.active, #media_content_browser_tabs .item-list a.active').removeClass('active');
        // Make this tab active
        $(this).addClass('active');
        $(this).parents('li').addClass('active');

        // Unload content in the subtabs
        $('#media_content_browser_subtabs').html();
        // Get content the subtabs for this tab
        renderSubTabs();
      });
    });
  }
}


/**
 * Load the subtabs based on what tab has been clicked
 * @param tabname
 * @param bundle
 * @param field_name
 * @return
 */
function renderSubTabs() {
  // Build our callback data
  var params = 'subtabs/'+$('#media_browser_data_store').attr('tabname');
  // We need to get the subtabs for the currently selected tab
  $.getJSON(build_callback_url(params),
    function(data) {
      $('#media_content_browser_subtabs').html(data);
      // Reattach behaviors to the content in the dialog box
      Drupal.attachBehaviors($('#dialog'));
      // Make the active subtab render its content
      renderSubTabPane();
    }
  );
}


/**
 *  This loads pane content based on subtab clicks. It needs
 *  to be run each time that a new set of sub tabs is rendered
 *  in the dialog box
 */
Drupal.behaviors.subTabsClick ={
  attach: function (context, settings) {
    $('#media_content_browser_subtabs li.vertical-tab-button a', context).once('subTabsClick', function() {
      $(this).bind('click', function () {
        // Store the current subtab identifier
        $('#media_browser_data_store').attr('identifier', $(this).attr('identifier'));
        renderSubTabPane();
      });
    });
  }
}


/**
 * This does the actual render of content into the subTab pane
 * We do not pass params to this so that it can be a generalized
 * function.
 * @TODO add a throbber to alert the user we are loading content
 */
function renderSubTabPane() {
   // The current active tab id is here
   var active_subtab = '#'+$('#edit-subtabs--active-tab').attr('value');
   // Derive the identifier for this pane
   var identifier = $(active_subtab+' input.subtab_data').attr('identifier');

   // Now we have to build a query which the dispatcher uses to get the correct content
   // for this subtab pane
   var query = '?identifier='+identifier;
   // This populates the pane with the proper settings
   $.getJSON(build_callback_url('pane', query),
     function(data) {
       // Remove any exisiting pane content
       $('div.pane_content.active').removeClass('active').html('');
       $(active_subtab+' div.pane_content').html(data);
       // Make this pane active
       $(active_subtab+' div.pane_content').addClass('active');
       // Attach any subtab behaviors.
       Drupal.attachBehaviors($(active_subtab+' div.pane_content'));
     }
  );
}


/**
 * Build a callback url from the data store
 */
function build_callback_url(params, query) {
  var bundle = $('#media_browser_data_store').attr('bundle');
  var object_type = $('#media_browser_data_store').attr('object-type');
  var field_name = $('#media_browser_data_store').attr('field-name');
  var identifier = $('#media_browser_data_store').attr('identifier');
  var url = Drupal.settings.media.media_browser_content_load_url+'/'+object_type+'/'+bundle+'/'+field_name;

  // Do we have parameters to add?
  if (typeof(params) != 'undefined') {
    url += '/'+params;
  }
  // Any queries?
  if (typeof(query) != 'undefined') {
    url += query;
  }

  return url;
}


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