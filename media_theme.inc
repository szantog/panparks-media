<?php

// $Id$

/**
 * @info 
 * This file provides all the theming functions for the Media module
 */


/**
 * Display the media form when the tabs module is not present
 * @param array $tabs
 * @return string
 *   formated html
 */
function theme_media_no_tabs_display ($tabs) {  
  return "it would be a good idea to install tabs, huh.";
}


/**
 * controler function for displaying tabs
 * @param array $tabs
 *   array of tab id => array(title => tab name, content => $content, attributes => array(attributes))
 * @param array $container_attributes
 *   array of attributes for the tabs container
 * @return string
 */
function theme_media_tabs_display($tabs, $container_attributes = null) {
  // add the right JS file in
  drupal_add_js(path_to_theme() .'/scripts/jquery.tabs.pack.js');

  // we always add the tabs_container to the class definition
  $container_attributes['class'] .= ' tabs_container';

  // create the container
  $html[] = '<div '. drupal_attributes($container_attributes) .'>';

  // start building the list
  $html[] = '  <ul>';
  // build each tab
  foreach ($tabs as $id => $tab) {
    // make sure that there is data
    if ($id && $tab) {
      // by default we have a set of attributes
      if (! $tab['attributes']) { $tab['attributes'] = array('class' => 'jquery_tabs'); }
      $html[] = '    <li '. drupal_attributes($tab['attributes']) .'><a href="#'. $id .'"><span>'. $tab['title'] .'</span></a></li>';
      // now put the content into an array
      $contents[$id] = $tab['content'];
    }
  }
  $html[] = '  </ul>';

  // now build the content
  foreach ($contents as $id => $content) {
    // don't build tabs we don't have content for
    if ($tabs[$id]) {
      $html[] = '  <div id="'. $id .'"><div class="rounded">';
      $html[] = '    '. $content;
      // add a clear to make sure content pushes tab all the way open
      $html[] = '<br class="clear" />';
      $html[] = '  </div></div><!-- /'. $id .' -->';
    }
  }

  // now close the main div
  $html[] = '</div><!-- /tabs_container -->';

  return implode("\n", $html);
}