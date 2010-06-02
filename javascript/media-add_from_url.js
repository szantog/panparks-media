// $Id$

/**
 * @file
 * This creates an AJAX preview for the 'from_url' browser form.
 */

(function ($) {

/**
 * Change the preview on textfield change.
 */
Drupal.behaviors.mediaAddFromURLPreview = {
  attach: function (context, settings) {
    $('.media-add-from-url', context).once('mediaAddFromURLPreview', function () {
      $(this).bind('change', function () {
        $preset = $(this);
        if ($preset.val()) {
          $.getJSON(Drupal.settings.media.add_from_url_preview + '?url=' + $preset.val(), function (data) {
            // @todo: Check for errors.
            $('#media-add-from-url-preview').html(data.preview);
          });
        }
      });
    });
  }
};

})(jQuery);
