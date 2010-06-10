// $Id$

/**
 * @file
 * Javascript for the interface at admin/content/media and also for interfaces
 * related to setting up media fields and for media type administration.
 *
 * Basically, if it's on the /admin path, it's probably here.
 */

(function ($) {

/**
 * Functionality for the thumbnail display
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


/**
 * JavaScript for the Media types administrative form.
 */
Drupal.behaviors.mediaTypesAdmin = {
  attach: function (context) {
    if ($('.form-item-match-type', context).length == 0) {
      return;
    }
    // Toggle the 'other' text field on Match type.
    if ($('.form-item-match-type input:checked').val() != '0') {
      $('.form-item-match-type-other').hide();
    }
    $('.form-item-match-type input').change(function () {
      if ($(this).val() == '0') {
        $('.form-item-match-type-other').slideDown('fast');
      }
      else {
        $('.form-item-match-type-other').slideUp('fast');
      }
    });
  }
};


  
})(jQuery);

