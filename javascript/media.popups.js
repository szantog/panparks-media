/**
 * @file: Popup dialog interfaces for the media project.
 *
 * Drupal.media.popups.mediaBrowser
 *   Launches the media browser which allows users to pick a piece of media.
 *
 * Drupal.media.popups.mediaStyleSelector
 *  Launches the style selection form where the user can choose
 *  what format / style they want their media in.
 *
 */

(function ($) {

namespace('Drupal.media.popups');

/**
 * Media browser popup. Creates a media browser dialog.
 *
 * @param {function}
 *          onSelect Callback for when dialog is closed, received (Array
 *          media, Object extra);
 * @param {Object}
 *          options Array of options for the browser.
 */
Drupal.media.popups.mediaBrowser = function(onSelect, options) {
  options = $.extend({}, Drupal.media.popups.mediaBrowser.getDefaults(), options);
  // Create it as a modal window.
  var mediaIframe = Drupal.media.popups.getPopupIframe(options.src, 'mediaBrowser');
  // Attach the onLoad event
  mediaIframe.bind('load', options, options.onLoad);
  /**
   * Setting up the modal dialog
   */
  /**
   * Set up the button text
   */
  var ok = 'OK';
  var cancel = 'Cancel';
  var notSelected = 'You have not selected anything!';

  if (Drupal && Drupal.t) {
    ok = Drupal.t(ok);
    cancel = Drupal.t(cancel);
    notSelected = Drupal.t(notSelected);
  }

  // @todo: let some options come through here. Currently can't be changed.
  var dialogOptions = Drupal.media.popups.getDialogOptions();

  dialogOptions.buttons[ok] = function () {
    var selected = this.contentWindow.Drupal.media.browser.selectedMedia;
    if (selected.length < 1) {
      alert(notSelected);
      return;
    }
    onSelect(selected);
    $(this).dialog("close");
  };

  dialogOptions.buttons[cancel] = function () {
    $(this).dialog("close");
  };

  Drupal.media.popups.setDialogPadding(mediaIframe.dialog(dialogOptions));
  // Remove the title bar.
  mediaIframe.parents(".ui-dialog").find(".ui-dialog-titlebar").remove();
  return mediaIframe;
};

Drupal.media.popups.mediaBrowser.mediaBrowserOnLoad = function (e) {
  var options = e.data;
  if (!this.contentWindow || !this.contentWindow.Drupal.media.browser) {
    return;
  }
  // Check to see if new media has been successfully added (e.g. a file upload).
  var fid = this.contentWindow.Drupal.media.browser.mediaAdded();
  if (fid) {
    Drupal.media.popups.mediaBrowser.chooseMedia(this, fid);
  }
  else {
    this.contentWindow.Drupal.media.browser.launch(options);
  }
};

/**
 * Select newly added media and submit the dialog.
 */
Drupal.media.popups.mediaBrowser.chooseMedia = function (iframe, fid) {
  var ok = $(iframe).dialog('option', 'buttons')['OK'];
  var options = {conditions: JSON.stringify({'fid': fid})};
  var callback = function (data) {
    iframe.contentWindow.Drupal.media.browser.selectedMedia = [data.media[fid]];
    ok.call(iframe);
  };
  $.getJSON(Drupal.settings.basePath + 'media/browser/list', options, callback);
};

Drupal.media.popups.mediaBrowser.getDefaults = function() {
  return {
    src: Drupal.settings.media.browserUrl,
    onLoad: Drupal.media.popups.mediaBrowser.mediaBrowserOnLoad
  };
}

/**
 * Style chooser Popup. Creates a dialog for a user to choose a media style.
 *
 * @param mediaFile
 *          The mediaFile you are requesting this formatting form for.
 *          @todo: should this be fid?  That's actually all we need now.
 *
 * @param Function
 *          onSubmit Function to be called when the user chooses a media
 *          style. Takes one parameter (Object formattedMedia).
 *
 * @param Object
 *          options Options for the mediaStyleChooser dialog.
 */
Drupal.media.popups.mediaStyleSelector = function(mediaFile, onSelect, options) {
  var defaults = Drupal.media.popups.mediaStyleSelector.getDefaults();
  // @todo: remove this awful hack :(
  defaults.src = defaults.src.replace('-media_id-', mediaFile.fid);
  options = $.extend({}, defaults, options);
  // Create it as a modal window.
  var mediaIframe = Drupal.media.popups.getPopupIframe(options.src, 'mediaBrowser');
  // Attach the onLoad event
  mediaIframe.bind('load', options, options.onLoad);

  /**
   * Set up the button text
   */
  var ok = 'OK';
  var cancel = 'Cancel';
  var notSelected = 'Very sorry, there was an unknown error embedding media.';

  if (Drupal && Drupal.t) {
    ok = Drupal.t(ok);
    cancel = Drupal.t(cancel);
    notSelected = Drupal.t(notSelected);
  }

  // @todo: let some options come through here. Currently can't be changed.
  var dialogOptions = Drupal.media.popups.getDialogOptions();

  dialogOptions.buttons[ok] = function () {

    var formattedMedia = this.contentWindow.Drupal.media.formatForm.getFormattedMedia();
    if (!formattedMedia) {
      alert(notSelected);
      return;
    }
    onSelect(formattedMedia);
    $(this).dialog("close");
  };

  dialogOptions.buttons[cancel] = function () {
    $(this).dialog("close");
  };

  Drupal.media.popups.setDialogPadding(mediaIframe.dialog(dialogOptions));
  // Remove the title bar.
  mediaIframe.parents(".ui-dialog").find(".ui-dialog-titlebar").remove();
  return mediaIframe;
}

Drupal.media.popups.mediaStyleSelector.mediaBrowserOnLoad = function(e) {
}

Drupal.media.popups.mediaStyleSelector.getDefaults = function() {
  return {
    src: Drupal.settings.media.styleSelectorUrl,
    onLoad: Drupal.media.popups.mediaStyleSelector.mediaBrowserOnLoad
  };
}


/**
 * Generic functions to both the media-browser and style selector
 */

/**
 * Returns the commonly used options for the dialog.
 */
Drupal.media.popups.getDialogOptions = function() {
  return {
    buttons: {},
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
  };
};

/**
 * Created padding on a dialog
 *
 * @param jQuery dialogElement
 *  The element which has .dialog() attached to it.
 */
Drupal.media.popups.setDialogPadding = function(dialogElement) {
  // @TODO: Perhaps remove this hardcoded reference
  var horizontalPadding = 30;
  var verticalPadding = 30;

  dialogElement.width(dialogElement.dialog('option', 'width') - horizontalPadding)
  dialogElement.height(dialogElement.dialog('option', 'height') - verticalPadding);
};

/**
 * Get an iframe to serve as the dialog's contents. Common to both plugins.
 */
Drupal.media.popups.getPopupIframe = function(src, id, options) {
  var defaults = {width: '800px', scrolling: 'no'};
  var options = $.extend({}, defaults, options);

  return $('<iframe class="media-modal-frame"/>')
  .attr('src', src)
  .attr('width', options.width)
  .attr('id', id)
  .attr('scrolling', options.scrolling)
};

// end of closure
})(jQuery);
