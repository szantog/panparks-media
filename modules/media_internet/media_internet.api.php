<?php
// $Id$

/**
 * Implementors return an multidim array, keyed by a class name
 * with the following elements:
 *
 * - title
 * - logo (optional)
 * - hidden: bool If the logo should be shown on form. (optional)
 * - weight (optional)
 */
function hook_media_internet_providers() {
  return array(
    'youtube' => array(
      'title' => 'youtube',
      'logo' => 'youtube.jpg'
    ),
  );
}

?>