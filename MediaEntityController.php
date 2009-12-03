<?php

/**
 * Default implementation of DrupalEntityControllerInterface.
 *
 * This class can be used as-is by most simple entity types. Entity types
 * requiring special handling can extend the class.
 */
class MediaEntityController extends DrupalDefaultEntityController {

  protected $entityCache;
  protected $entityType;
  protected $entityInfo;
  protected $hookLoadArguments;
  protected $idKey;
  protected $revisionKey;
  protected $revisionTable;
  protected $query;

public function load($ids = array(), $conditions = array()) {
    $this->ids = $ids;
    $this->conditions = $conditions;

    $entities = array();

    // Revisions are not statically cached, and require a different query to
    // other conditions, so separate the revision id into its own variable.
    if ($this->revisionKey && isset($this->conditions[$this->revisionKey])) {
      $this->revisionId = $this->conditions[$this->revisionKey];
      unset($this->conditions[$this->revisionKey]);
    }
    else {
      $this->revisionId = FALSE;
    }


    // Create a new variable which is either a prepared version of the $ids
    // array for later comparison with the entity cache, or FALSE if no $ids
    // were passed. The $ids array is reduced as items are loaded from cache,
    // and we need to know if it's empty for this reason to avoid querying the
    // database when all requested entities are loaded from cache.
    $passed_ids = !empty($this->ids) ? array_flip($this->ids) : FALSE;
    // Try to load entities from the static cache, if the entity type supports
    // static caching.
    if ($this->cache) {
      $entities += $this->cacheGet($this->ids, $this->conditions);
      // If any entities were loaded, remove them from the ids still to load.
      if ($passed_ids) {
        $this->ids = array_keys(array_diff_key($passed_ids, $entities));
      }
    }

    // Load any remaining entities from the database. This is the case if $ids
    // is set to FALSE (so we load all entities), if there are any ids left to
    // load, if loading a revision, or if $conditions was passed without $ids.
    if ($this->ids === FALSE || $this->ids || $this->revisionId || ($this->conditions && !$passed_ids)) {
      // Build the query.
      $this->buildQuery();
      $queried_entities = $this->query
        ->execute()
        ->fetchAllAssoc($this->idKey);
    }

    foreach ($queried_entities as &$entity) {
      $entity->type = self::getBundleName($entity->filemime);
    }
    // Pass all entities loaded from the database through $this->attachLoad(),
    // which attaches fields (if supported by the entity type) and calls the
    // entity type specific load callback, for example hook_node_load().
    if (!empty($queried_entities)) {
      $this->attachLoad($queried_entities);
      $entities += $queried_entities;
    }

    if ($this->cache) {
      // Add entities to the cache if we are not loading a revision.
      if (!empty($queried_entities) && !$this->revisionId) {
        $this->cacheSet($queried_entities);
      }
      // Ensure that the returned array is ordered the same as the original
      // $ids array if this was passed in and remove any invalid ids.
      if ($passed_ids) {
        // Remove any invalid ids from the array.
        $passed_ids = array_intersect_key($passed_ids, $entities);
        foreach ($entities as $entity) {
          $passed_ids[$entity->{$this->idKey}] = $entity;
        }
        $entities = $passed_ids;
      }
    }

    return $entities;
  }

  public static function getBundleName($mime) {
    $types = module_invoke_all('media_types');
    $name = substr($mime, 0,strpos($mime, '/'));
    if (in_array($name, array_keys($types))) {
      return $name;
    } else {
      return FALSE;
    }
    // @todo: make this more flexible.
//    $types = module_invoke_all('media_types');
//    foreach ($types as $type) {
//      if (preg_match($type->mimeTypes, ) {
//
//      }
//
//    }
  }
}