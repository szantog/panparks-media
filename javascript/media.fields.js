(function ($) {
/**
 * Loads media browsers and callbacks, specifically for media as a field.
 */
  
Drupal.behaviors.mediaBrowserFields ={
  attach: function (context, settings) {
    // For each media field
    $('.field-type-media', context).each(function () {
      var options = Drupal.settings.media.fields[this.id];
      // For each widget
      $('.media-widget', this).once('mediaBrowserLaunch', function () {

        //options = Drupal.settings.media.fields[this.id];
        var fidField = $('.fid', this);
        var previewField = $('.preview', this);
        
        if (Drupal.settings.media.debug) {
          // @TODO: Remove this, here for debugging purposes.
          fidField.show();
          var debugField = $('.file_info', this);
        }

        // When someone clicks the link to pick media:
        $('.launcher', this).bind('click', function () {
          // Launch the browser, providing the following callback function
          // @TODO: This should not be an anomyous function.
          Drupal.media.popups.mediaBrowser(function (mediaFiles) {
            if (mediaFiles.length < 0) {
              return;
            }
            var mediaFile = mediaFiles[0];
            // Set the value of the filefield fid (hidden).
            fidField.val(mediaFile.fid);
            // Set the preview field HTML
            previewField.html(mediaFile.preview);
            if (Drupal.settings.media.debug) {
              // @TODO: Remove this, here for debugging purposes.
              debugField.html(JSON.stringify(mediaFile));
            }
          }, options);
          return false;
        });
      });
    });
  }
};
})(jQuery);