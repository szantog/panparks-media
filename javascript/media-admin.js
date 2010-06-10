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
            link.parents('.media-display-thumbnails').find('div.media-thumbnail :checkbox').attr('checked', true).change();
            break;
          case Drupal.t('none'):
            link.parents('.media-display-thumbnails').find('div.media-thumbnail :checkbox').attr('checked', false).change();
            break;
        }
        return false;
      });

    // Add an extra class to selected thumbnails.
    $('li.media-item :checkbox', '.media-display-thumbnails').each(function () {
      var checkbox = $(this);
      if (checkbox.is(':checked')) {
        $('img', checkbox.parents('li.media-item')).addClass('selected');
      }
      checkbox.change(function () {
        if (checkbox.is(':checked')) {
          $('img', checkbox.parents('li.media-item')).addClass('selected');
        }
        else {
          $('img', checkbox.parents('li.media-item')).removeClass('selected');
        }
      });
    });
    var fieldset = $('#edit-options');
    // Only show update options if anything gets checked.
    $('input[type=checkbox]').bind('change.media', function () {
      if (!$('input[type=checkbox]:checked').size()) {
        fieldset.slideUp('fast');
      }
      else {
        fieldset.slideDown('fast');
      }
    });
    fieldset.hide();
    
  }
};
  
})(jQuery);
