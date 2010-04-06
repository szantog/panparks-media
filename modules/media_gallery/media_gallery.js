// $Id$

(function ($) {
  Drupal.behaviors.mediaGalleryWidget = {
    attach: function (context) {
      // On the edit form.
      $('.media-widget-closed', context).once('mediaWidgetDrawrerDone', this.processWidget);
    },

    processWidget: function() {
      var $this = $(this);
      var $wrapper = $('.gallery-fields-wrapper', this);
      $wrapper.slideUp();
        $('.edit-description-link', this).bind('click', function() {
          $wrapper.slideDown();
          $this.addClass('media-widget-open');
          $this.removeClass('media-widget-closed');
        });
    }
  };

  Drupal.behaviors.mediaGallerySlideshow = {
    attach: function (context) {
      // Temporary jCycle code (needs to be abstracted, with settings, etc)
      $('.field-items').once('jcycleActivated', function () {
        $(this).cycle({
          fx: 'fade',
          timeout: 6000,
          delay: -2000
        }
        );
      });
    }
  };
})(jQuery);