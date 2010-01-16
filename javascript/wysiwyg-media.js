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
      tagContent = Drupal.wysiwyg.plugins.media.createTag(embeddedMedia);
      // When tagmap is defined such as node/edit, block/configure
      if(Drupal.settings.tagmap) {
        Drupal.settings.tagmap[tagContent] = Drupal.wysiwyg.plugins.media.addWrapper(embeddedMedia.html());
      }
      // When tagmap is not defined such as node/add/, block/add
      else {
        Drupal.settings.tagmap = { };
        Drupal.settings.tagmap[tagContent] = Drupal.wysiwyg.plugins.media.addWrapper(embeddedMedia.html());
      }
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
    	tagContent = Drupal.wysiwyg.plugins.media.createTag(this);
        $(this).replaceWith(tagContent);
      });
      return content.html();
    },
    
    addWrapper: function(htmlContent) {
      return '<div class="media-embedded">' + htmlContent + '</div>';  
    },
    
    createTag: function(mediaObj) {
        var imgNode = $("img",mediaObj);
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
        return '[[' + JSON.stringify(tagContent) + ']]';
      }
  };
  
  Drupal.media = Drupal.media || {};
  Drupal.media.formatForm = {
    launch: function(mediaFile, callback) {
    
      // Really, we should be doing what's happening below.
      // For now, we're just retutning the preview
    
      callback(mediaFile.preview);
      return;
    
      $('<div id="format_form"></div>')
        .load(Drupal.settings.basePath + 'media/' + mediaFile.fid + '/format-form')
        .dialog({
          buttons: { 
            "Ok": function() {
              callback(mediaFile.preview);
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
        });
    }
  }

})(jQuery);
