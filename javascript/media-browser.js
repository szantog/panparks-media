
(function ($) {

/**
 * Quick hacked up media browser for testing communication
 */
Drupal.behaviors.mediaBrowser = {
  attach: function(context, settings) {
    
  }
};

Drupal.mediaBrowser = Drupal.mediaBrowser || {};
Drupal.mediaBrowser.selectedMedia = [];

Drupal.mediaBrowser.defaults = {
  viewType: 'thumbnails',
  //@todo: use variable
  callback: '/media/browser/list',
  conditions: {}
};
  
Drupal.mediaBrowser.start = function(options) {
  this.settings = $.extend({}, this.defaults, options);
  this.browser = $('#media-browser-list tbody');
  this.getMedia();
};

Drupal.mediaBrowser.getCallbackUrl = function() {
  return this.settings.callback;
}

Drupal.mediaBrowser.getConditions = function () {
  return this.settings.conditions;
},

Drupal.mediaBrowser.getMedia = function () {
  var that = this;
  var callback = this.getCallbackUrl();
  var params = {
    conditions: this.getConditions()
  };
  
  jQuery.get(
    callback,
    params,
    function(data, status) {
      that.mediaFiles = data;
      that.refresh();
    },
    'json'
  );
},

Drupal.mediaBrowser.refresh = function () {
  var that = this;
  //this.browser.html('');
  
  for (m in this.mediaFiles) {
    mediaFile = this.mediaFiles[m];
    // Create a wrapper div for selecting
    this.browser.append(
      jQuery('<tr></tr>').append(
        jQuery('<td></td>')
          .attr('id', 'media-file-' + mediaFile.fid)
          .addClass('media-file')
          .html(mediaFile.preview)
          .bind('click', mediaFile, function(e) {
            // Notify the main browser
            //this.selectedMedia = mediaFile;
            $('.media-file').parent().removeClass('selected');
            $(this).parent().addClass('selected');
            that.selectedMedia.push(e.data);
            //sandbox.notify('browser.mediaSelected', {mediaFile: mediaFile});
            //that.settings.onSelect(mediaFile);
            return false;
          })
        ).append(
        jQuery('<td></td>')
          .html(JSON.stringify(mediaFile))
      )
    );
    //onSelect
  }
};

})(jQuery);

//
//  var dummy = {
//   fid: 1,
//   uri: 'http://bs.com/bs.png',
//   preview: '<span> I am a preview </span>'
//  };
//  this.selectedMedia.push(dummy);
