
/**
 * @file: jQuery plugin for mediaBrowser
 */
(function ($) {

/**
   * @param function onSelect
   *   Callback for when dialog is closed, received (Array media, Object extra);
   * @param Object options
   *   Array of options for the browser.
   */
  $.fn.mediaBrowser = function(onSelect, options) {
   options = $.extend({}, $.fn.mediaBrowser.getDefaults(), options);
   debug.debug(options);

   this.each(function() {
   // Create it as a modal window.
    var mediaIframe = $.fn.mediaBrowser.getIframe(options.src);
      mediaIframe.bind('load', function (e) {
        if (typeof this.contentWindow.Drupal.mediaBrowser != undefined) {
          this.contentWindow.Drupal.mediaBrowser.start(options);
        }
        debug.debug('browser is loaded');
      });

    var horizontalPadding = 30;
    var verticalPadding = 30;
    mediaIframe.dialog({
      buttons: { "Ok": function() {
        var selected = this.contentWindow.Drupal.mediaBrowser.selectedMedia;
        onSelect(selected);
        $(this).dialog("close");
        }
      },
      modal: true,
      draggable: true,
      resizable: true,
      minWidth: 600,
      width: 800,
      height:500,
      position: 'top',
      overlay: {
        backgroundColor: '#000000',
        opacity: 0.4
      }
      })
      .width(800 - horizontalPadding)
      .height(500 - verticalPadding);
   });
   return this;
  };


  $.fn.mediaBrowser.getDefaults = function () {
    debug.debug(Drupal.settings.media);
    return {
      media_types: ['video', 'images'],
      src: Drupal.settings.media.browserUrl
    };
  };

  $.fn.mediaBrowser.getIframe = function (src) {
    return $('<iframe id="mediaBrowser" class="media-modal-frame"/>')
    .attr('src', src)
    .attr('width', '800px');
  }
// end of closure
})(jQuery);
