(function ($) {
/**
 * Loads media browsers and callbacks, specifically for media as a field.
 */
  
Drupal.behaviors.mediaBrowserFields ={
  attach: function (context, settings) {
    // For each media field
    $('.field-type-media', context).once('mediaBrowserLaunch', function () {
      //options = Drupal.settings.media.fields[this.id];
      
      //debug.debug(this.id);
      //debug.debug(options);

      var fidField = $('.fid', $(this));
      var previewField = $('.preview', $(this));
      
      if (Drupal.settings.media.debug) {
        // @TODO: Remove this, here for debugging purposes.
        fidField.show();
        var debugField = $('.file_info', this);
      }
      
      // When someone clicks the link to pick media:
      $('.launcher', this).bind('click', function () {
        var options = {};
        // Launch the browser, providing the following callback function
        // @TODO: This should not be an anomyous function.
        Drupal.media.popups.mediaBrowser(function (mediaFiles, options) {
          if (mediaFiles.length < 0) {
            return;
          }
          var mediaFile = mediaFiles[0];
          // Set the value of the filefield fid (hidden).
          fidField.val(mediaFile.fid);
          // Set the preview field HTML
          debug.debug(mediaFile.preview);
          previewField.html(mediaFile.preview);
          if (Drupal.settings.media.debug) {
            // @TODO: Remove this, here for debugging purposes.
            debugField.html(JSON.stringify(mediaFile));
          }
        });
        return false;
      });
    });
  }
};
})(jQuery);