

(function ($) {
namespace('Drupal.media.browser');

Drupal.behaviors.mediaLibrary = {
  attach: function (context, settings) {
    var library = new Drupal.media.browser.library(Drupal.settings.media.browser.library);
    $('#media-browser-tabset').bind('tabsselect', function(event, ui) {
      if (ui.tab.hash === '#media-tab-library') {
        // @todo: implement the types param here.
        var params = {};
        params.types = Drupal.settings.media.browser.library.types

        //$(ui.panel).addClass('throbber');
        library.reset($(ui.panel));
        library.start($(ui.panel), params);
      }
    });
  }
}

Drupal.media.browser.library = function (settings) {
  this.settings = Drupal.media.browser.library.getDefaults();
  $.extend(this.settings, settings);

  this.done = false; // Keeps track of if the last request for media returned 0 results.
  
  this.cursor = 0; // keeps track of what the last requested media object was.
  this.mediaFiles = [] // An array of loaded media files from the server.
}

Drupal.media.browser.library.getDefaults = function() {
  return {
    emtpyMessage: "There is nothing in your media library.  Select the Upload tab above to add a file.",
    limit: 15
  }
}


Drupal.media.browser.library.prototype.reset = function (renderElement) {
  this.cursor = 0;
  this.mediaFiles = [];
  $('#media-browser-library-list', renderElement).html('');
}

Drupal.media.browser.library.prototype.start = function(renderElement, params) {
  this.renderElement = renderElement;
  this.params = params;
  //this.loadMedia();
  this.scrollUpdater();
}

/**
 * Appends more media onto the list
 */
Drupal.media.browser.library.prototype.loadMedia = function() {
  var that = this;
  $('#status').text('Loading...');
  $.extend(this.params, {start: this.cursor, limit: this.settings.limit});
  
  var gotMedia = function (data, status) {
    $('#status').text('');
    if (data.media.length == 0) {
      // we're at the end of the line
      clearTimeout(that.updaterTimeout);
      // There must be a better way to do this.
      //that.done = true;
      return;
    }
    that.mediaFiles = that.mediaFiles.concat(data.media);
    that.render(that.renderElement);
  }

  var errorCallback = function() {
    alert('Error getting media.');
    clearTimeout(that.updaterTimeout);
  }

  jQuery.ajax({
    url: this.settings.getMediaUrl,
    type: 'GET',
    dataType: 'json',
    data: this.params,
    error: errorCallback,
    success: gotMedia
  });
}

Drupal.media.browser.library.prototype.scrollUpdater = function(){
	var scrolltop = $('#scrollbox').attr('scrollTop');
	var scrollheight = $('#scrollbox').attr('scrollHeight');
	var windowheight = $('#scrollbox').attr('clientHeight');
	var scrolloffset = 20;
  
	if(scrolltop >= (scrollheight - (windowheight + scrolloffset)))
	{
		//fetch new items
    this.loadMedia();
	}
  var that = this;
	this.updaterTimeout = setTimeout(function() {that.scrollUpdater()}, 1500);
}

/**
 * Fetches the next media object and increments the cursor.
 */
Drupal.media.browser.library.prototype.getNextMedia = function() {
  if (this.cursor >= this.mediaFiles.length) {
    return false;
  }
  var ret = this.mediaFiles[this.cursor];
  this.cursor += 1;
  return ret;
}

Drupal.media.browser.library.prototype.render = function (renderElement) {
  var that = this;

  if (this.mediaFiles.length < 1) {
    jQuery('<div id="media-empty-message" class="media-empty-message"></div>').appendTo(renderElement)
      .html(this.emptyMessage);
    return;
  } else {
    var mediaList = $('#media-browser-library-list', renderElement);
    // If the list doesn't exist, bail.
    if (mediaList.length === 0) {
      throw('Cannot continue, list element is missing');
    }
  }

  while(1) {
    var mediaFile = this.getNextMedia();
    if (mediaFile === false) {
      break;
    }

    var listItem = jQuery('<li></li>').appendTo(mediaList)
      .attr('id', 'media-file-' + mediaFile.fid)
      .addClass('media-file');

    var imgLink = jQuery('<a href="#"></a>').appendTo(listItem)
      .html(mediaFile.preview)
      .bind('click', mediaFile, function(e) {
        // Notify the main browser
        //this.selectedMedia = mediaFile;
        $('div.media-thumbnail img').removeClass('selected');
        $('div.media-thumbnail img', $(this)).addClass('selected');
        that.mediaSelected([e.data]);
        //that.settings.onSelect(mediaFile);
        return false;
      });
  }
}

Drupal.media.browser.library.prototype.mediaSelected = function(media) {
  Drupal.media.browser.selectMedia(media);
}

}(jQuery));

