// $Id$

/**
 * @file
 * Handles the JS for Media module functions.
 */

(function ($) {

/**
 * Thumbnails display
 */
Drupal.behaviors.mediaAdmin = {
  attach: function (context) {
    // Implements 'select all/none'.
    $('<div class="media-thumbnails-select" />')
      .append('<strong>' + Drupal.t('Select') + ':</strong> <a href="#">' + Drupal.t('all') + '</a>, <a href="#">' + Drupal.t('none') + '</a>')
      .prependTo('.media-display-thumbnails')
      .find('a')
      .click(function () {
        var link = $(this);
        switch ($(this).text()) {
          case Drupal.t('all'):
            link.parents('.media-display-thumbnails').find('div.media-thumbnail :checkbox').attr('checked', true);
            break;
          case Drupal.t('none'):
            link.parents('.media-display-thumbnails').find('div.media-thumbnail :checkbox').attr('checked', false);
            break;
        }
        return false;
      });

    // Add an extra class to selected thumbnails.
    $('div.media-thumbnail :checkbox', '.media-display-thumbnails').each(function () {
      var checkbox = $(this);
      if (checkbox.is(':checked')) {
        checkbox.parents('div.media-thumbnail').addClass('media-thumbnail-checked');
      }
      checkbox.change(function () {
        if (checkbox.is(':checked')) {
          checkbox.parents('div.media-thumbnail').addClass('media-thumbnail-checked');
        }
        else {
          checkbox.parents('div.media-thumbnail-checked').removeClass('media-thumbnail-checked');
        }
      });
    }); 
  }
};
  
})(jQuery);
