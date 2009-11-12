// $Id$

/**
 *  @file
 *  This file handles the JS for Media Module functions.
 */

(function ($) {


/* **************************************************** */
/* Content browser navigation                           */
/* **************************************************** */


/**
 *  This loads pane content based on subtab clicks. It needs
 *  to be run each time that a new set of sub tabs is rendered 
 *  in the dialog box
 */
Drupal.behaviors.subTabsPaneLoad ={
  attach: function (context, settings) {
    $('#media_content_browser_subtabs li.vertical-tab-button  a', context).once('subTabsPaneLoad', function() {
      $(this).bind('click', render_subTab_pane_content());
    });
  }
}


/**
 * This does the actual render of content into the subTab pane
 * @TODO add a throbber to alert the user we are loading content
 */
function render_subTab_pane_content() {
   // @TODO we have to pull in the pane content via ajax when the active tab is clicked
   // we can find it with something like:
   var active_tab = '#'+$('#edit-subtabs--active-tab').attr('value');
   var bundle = $(active_tab+' input').attr('bundle');
   var object_type = $(active_tab+' input').attr('object-type');
   var field_name = $(active_tab+' input').attr('field-name');
   
   // Now we have to build a query which the dispatcher uses to get the correct content
   // for this subtab
   var query = '?module='+$(active_tab+' input').attr('module')+'&identifier='+$(active_tab+' input').attr('identifier');
   // This populates the pane with the proper settings
   $.getJSON(Drupal.settings.media.media_browser_content_load_url+'/'+object_type+'/'+bundle+'/'+field_name+'/pane'+query,
     function(data) { 
       // Remove any exisiting pane content
       $('div.pane_content.active').removeClass('active').html('');
       $(active_tab+' div.pane_content').html(data);
       // Make this pane active
       $(active_tab+' div.pane_content').addClass('active');
     }
   );
   
   // @TODO we need to clear out all previous data from closed panes here
}

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
  $('input.media-file-uri ').bind('click', function() {
           
    // Add the dialog box to the page using this file field id
    media_render_dialog($(this).attr('id'), $(this).attr('object-type'), $(this).attr('bundle'), $(this).attr('field-name'));

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
        opacity: 0.4
      }
    });
    

    // Get the current query string    
  //  var query = $(this).attr('href').replace(/.*\?/, '')+'&file_field_id='+$(this).attr('id');
    // Load our content from ajax and style 
  //  media_load_content_display(query);
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
  
   
   /**
    * Render the modal dialog box
    */
   function media_render_dialog(field_id, object_type, bundle, field_name) {
     // If the dialog box already exists, we need to remove it from the DOM
     if ($('#dialog.ui-dialog-content')) {
       $('#dialog').dialog('destroy');
       $('#dialog').remove('#dialog');
     }
     // Build out the full HTML structure for the dialog box
     var html = '<div id="dialog" style="display: none;" title="Select your files"> \
       <div id="media_content_browser_throbber"></div> \
       <div id="media_content_browser_tabs"></div> \
       <div id="media_content_browser_subtabs"></div> \
     </div>';
   
     // Add the html to the page
     $('#'+field_id).append(html);
          
     // We need to get the tabs for this dialog box. Note that
     // these are static
     $.getJSON(Drupal.settings.media.media_browser_content_load_url+'/'+object_type+'/'+bundle+'/'+field_name+'/tabs',
       function(data) { $('#media_content_browser_tabs').html(data);}
     );

     // We need to get the subtabs for the currently selected tab
     // @TODO this needs to be dynamic based on above
     $.getJSON(Drupal.settings.media.media_browser_content_load_url+'/'+object_type+'/'+bundle+'/'+field_name+'/subtabs',
       function(data) {
         $('#media_content_browser_subtabs').html(data);
         // Reattach behaviors to the content in the dialog box
         Drupal.attachBehaviors($('#dialog'));
       }       
     );
   }
   
}); // $(document).ready


})(jQuery);