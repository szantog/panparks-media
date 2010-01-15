(function ($) {
  
  Drupal.wysiwyg.plugins.media = {
    /**
     * Execute the button.
     */
    invoke: function(data, settings, instanceId) {
      if (data.format == 'html') {
        $().mediaBrowser( function (mediaFiles) {
          var mediaFile = mediaFiles[0];
          //@todo: turn this into a non-anonymous bind
          Drupal.media.formatForm.launch(mediaFile, function(formattedMedia) {
            Drupal.wysiwyg.plugins.media.insertMediaFile(mediaFile, formattedMedia, Drupal.wysiwyg.instances[instanceId]);
          });
        });
      }    
    },
    
    insertMediaFile: function(mediaFile, formattedMedia, wysiwygInstance) {
      // Hack to allow for use of .html()
      var embeddedMedia = $('<div>' + formattedMedia + '</div>');
      // add the fid attribute to the image
      $('img', embeddedMedia).attr('fid', mediaFile.fid);
      wysiwygInstance.insert(Drupal.wysiwyg.plugins.media.addWrapper(embeddedMedia.html()));
    },
    
    /**
     * Atach function, called when a rich text editor loads.
     * This finds all [[tags]] and replaces them with the html
     * that needs to show in the editor.
     * 
     */
    attach: function(content, settings, instanceId) {    
      matches = content.match(/\[\[.*?\]\]/g);
  	tagmap = Drupal.settings.tagmap;
  	console.debug(tagmap);
  	  if(matches) {
  	    for (i=0; i< matches.length; i++) {
  		  for (var tagContent in tagmap ) {
  		    if (tagContent === matches[i]) {
  			  // This probably needs some work...
  			  // We need to somehow get the fid propogated here.
  			  // We really want to
  			  matches[i] = matches[i].replace('[[','');
  			  matches[i] = matches[i].replace(']]','');
  			  mediaObj = JSON.parse(matches[i]);
  			  imgMarkup = $(tagmap[tagContent]);
  			  $('img',imgMarkup).attr('fid',mediaObj.fid);
  			  content = content.replace(tagContent,this.addWrapper(imgMarkup.html()));
  			}
  		  }
  	    }
  	  }
  	  return content;
    },
  
    /**
     * Detach function, called when a rich text editor detaches
     */
    detach: function(content, settings, instanceId) {
      var content = $('<div>' + content + '</div>');
      $('div.media-embedded',content).each(function (elem){
        var imgNode = $("img",this);
        tagContent = {
          "type": 'media',
        	//@todo: This will be selected from the format form
        	"view_mode": 'media_original',
        	"fid" : imgNode.attr('fid'),
        	"attributes": {
        	  "width" : imgNode.attr('width'),
        	  "height" : imgNode.attr('height')
        	}
        };
        tagContent = '[[' + JSON.stringify(tagContent) + ']]';
        $(this).replaceWith(tagContent);
      });
      return content.html();
    },
    
    addWrapper: function(htmlContent) {
      return '<div class="media-embedded">' + htmlContent + '</div>';  
    }
  };
  
  Drupal.media = Drupal.media || {};
  Drupal.media.formatForm = {
    launch: function(mediaFile, callback) {
      callback($(mediaFile.preview).html());
      //$('<div id="format_form"></div>').dialog()
    }
  }

})(jQuery);
