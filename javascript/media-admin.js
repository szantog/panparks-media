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

    // Configure the "Add file" link to fire the media browser popup.
    $('ul.action-links li', context).remove();
    var $launcherLink = $('<a class="media-launcher" href="#"></a>').html('Add file');
    $launcherLink.bind('click', function () {
      // This option format needs *serious* work.
      // Not even bothering documenting it because it needs to be thrown.
      // See media.browser.js and media.browser.inc - media_browser()
      // For how it gets passed.
      var options = {
        disabledPlugins: ['library']
      };
      Drupal.media.popups.mediaBrowser(function (mediaFiles) {
        window.location.reload();
        return false;
      }, options);
    });

    
    $('ul.action-links', context).append($('<li></li>').append($launcherLink));

    // Implements 'select all/none'.
    $('<div class="media-thumbnails-select" />')
      .append('<strong>' + Drupal.t('Select') + ':</strong> <a href="#">' + Drupal.t('all') + '</a>, <a href="#">' + Drupal.t('none') + '</a>')
      .prependTo('.media-display-thumbnails')
      .find('a')
      .click(function () {
        var link = $(this);
        switch ($(this).text()) {
          case Drupal.t('all'):
            link.parents('.media-display-thumbnails').find(':checkbox').attr('checked', true).change();
            break;
          case Drupal.t('none'):
            link.parents('.media-display-thumbnails').find(':checkbox').attr('checked', false).change();
            break;
        }
        return false;
      });

    $('.media-item').bind('click', function (e) {
      if ($(e.target).is('img, a')) {
        return;
      }
      var checkbox = $(this).parent().find(':checkbox');
      if (checkbox.is(':checked')) {
        checkbox.attr('checked', false).change();
      } else {
        checkbox.attr('checked', true).change();
      }
    });

    // Add an extra class to selected thumbnails.
    $('.media-display-thumbnails :checkbox').each(function () {
      var checkbox = $(this);
      if (checkbox.is(':checked')) {
        $(checkbox.parents('li').find('.media-item')).addClass('selected');
      }
      checkbox.bind('change.media', function () {
        if (checkbox.is(':checked')) {
          $(checkbox.parents('li').find('.media-item')).addClass('selected');
        }
        else {
          $(checkbox.parents('li').find('.media-item')).removeClass('selected');
        }

        var fieldset = $('#edit-options');
        if (!$('input[type=checkbox]:checked').size()) {
          fieldset.slideUp('fast');
        }
        else {
          fieldset.slideDown('fast');
        }
      });
    });
   
    $('#edit-options').hide();

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

