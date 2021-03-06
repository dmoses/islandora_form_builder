<?php

// $Id$

/*
 * @file
 *
 */
module_load_include('inc', 'islandora_form_builder', 'Utilities');
module_load_include('inc', 'islandora_form_builder', 'FormBuilderInterface');

/**
 *
 */
class IngestFormBuilder implements FormBuilderInterface {

  /**
   * Drupal persistance form storage, specifically the index STORAGE_KEY where all
   * persistant data related to the islandora_form_builder is kept.
   *
   * @var array
   */
  protected $storage;
  /**
   * The pid of the collection that an object will be ingest into. Or the
   * collection in which the object to be edited belongs to.
   *
   * @var string
   */
  protected $collectionPid;
  /**
   * The pid of the content model that will be used to create the object. This is
   * only availible after the first form has been validated.
   *
   * @var string
   */
  protected $contentModelPid;
  /**
   * The ID of the XML datastream that the second form repersents. This is
   * only availible after the first form has been validated.
   *
   * @var string
   */
  protected $contentModelDsid;
  /**
   * The label for the collection identified by $collectionPid. Really only used to show the users what
   * collection they are dealing with.
   *
   * @var string
   */
  protected $collectionLabel;
  /**
   * The name of the ingest form to be build. This gets set on the first page of the form.
   *
   * @var string
   */
  protected $formName;
  /**
   * The current page of this multi-page form.
   *
   * @var integer
   */
  protected $page;
  /**
   * Drupal form state
   *
   * @var array
   */
  protected $formState;

  /**
   * Create the Ingest Form Builder.
   *
   * @param array $form_state
   * @param string $collection_pid
   * @param string $collection_label
   */
  public function __construct(&$form_state, $collection_pid = NULL, $collection_label = NULL) {
    $this->initializeStorage($form_state, $collection_pid, $collection_label);
    $this->formState = &$form_state;
    $this->storage = &$this->formState['storage'][STORAGE_KEY];
    $this->collectionPid = $this->storage['collection_pid'];
    $this->collectionLabel = $this->storage['collection_label'];
    $this->contentModelPid = isset($this->storage['content_model_pid']) ? $this->storage['content_model_pid'] : NULL;
    $this->contentModelDsid = isset($this->storage['content_model_dsid']) ? $this->storage['content_model_dsid'] : NULL;
    $this->formName = isset($this->storage['form_name']) ? $this->storage['form_name'] : NULL;
    $this->page = &$this->storage['ingest_form_page'];
  }

  /**
   * Initialize the form's persistant storage.
   *
   * @param array $form_state
   * @param string $collection_pid
   * @param string $collection_label
   */
  private function initializeStorage(&$form_state, $collection_pid = NULL, $collection_label = NULL) {
    if (!isset($form_state['storage'][STORAGE_KEY]) || !is_array($form_state['storage'][STORAGE_KEY])) {
      $form_state['storage'][STORAGE_KEY] = array();
    }
    $storage = &$form_state['storage'][STORAGE_KEY];
    $storage['collection_pid'] = isset($storage['collection_pid']) ? $storage['collection_pid'] : $collection_pid;
    $storage['collection_label'] = isset($storage['collection_label']) ? $storage['collection_label'] : $collection_label;
    if (empty($storage['ingest_form_page'])) { // First page by default.
      $storage['ingest_form_page'] = 1;
    }
  }

  /**
   * Gets the ingest form for rendering.
   *
   * @return array
   *   Returns a Drupal form if the user is allowed to ingest and the form exists in the content model,
   *   otherwise it returns FALSE.
   */
  public function createForm() {
    module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
    if (!$this->canIngest()) {
      return FALSE;
    }
    return $this->getForm();
  }

  /**
   * Tests to see if the user can ingest.
   *
   * @return boolean
   *   TRUE if the user can ingest FALSE otherwise.
   */
  private function canIngest() {
    if (!user_access('ingest new fedora objects')) { // Drupal Security
      drupal_set_message(t('You do not have permission to ingest.'), 'error');
      return FALSE;
    }
    else if (!$this->canIngestObjectIntoCollection($this->collectionPid)) { // Fedora security
      drupal_set_message(t('You do not have premission to ingest here.'));
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Tests if the user can ingest into the collection specified by $collection_pid
   *
   * Queries the collection object for a childsecurity datastream and if found parses it
   * to determine if this user is allowed to ingest in this collection we assume if
   * they are able to modify objects in the collection they can ingest as well.
   *
   * @param string $collection_pid
   *   The pid of the collection that will be queried to see if ingest is possible.
   *
   * @return boolean
   *   True if the user can ingest an object into the collection,
   *   specified by $collection_pid
   */
  private function canIngestObjectIntoCollection($collection_pid) {
    module_load_include('inc', 'fedora_repository', 'SecurityClass');
    $securityClass = new SecurityClass();
    return $securityClass->canIngestHere($collection_pid);
  }

  /**
   * Gets the appropriate ingest form, based on the forms page number.
   *
   * @return array
   *   The Drupal form to be rendered.
   */
  private function getForm() {
    return $this->page == 1 ?
        $this->getFormPageOne() :
        $this->getFormPageTwo();
  }

  /**
   * Gets the first page of the ingest form.
   *
   * The first page of the ingest form allows the user to select the Content Model, they want the ingest object
   * to have. It also allows the user to select which ingest form they want to use, if more than one exist. Optionally
   * in the Content modeler a particular form may be forced in that case the other forms will not be listed as options.
   *
   * @return array
   *   The first page of the ingest form.
   */
  private function getFormPageOne() {
    $collection_policy = CollectionPolicy::loadFromCollection($this->collectionPid);
    if ($collection_policy === FALSE) {
      drupal_set_message(t('Unable to load collection policy \'' . $this->collectionPid . '\'.'), 'error');
      return FALSE;
    }
    $content_models = $collection_policy->getContentModels();
    if (!$content_models) {
      $error_msg = 'No content models associated with this collection: !collection_label. Please contact your administrator.';
      drupal_set_message(t($error_msg, array('!collection_label' => $this->collectionLabel)), 'error');
      return FALSE;
    }
    $models_for_form = array();
    $forms_avaliable = array();
    foreach ($content_models as $content_model) {
      $identifier = $content_model->getIdentifier();
      $name = $content_model->name;
      $models_for_form[$identifier] = $name;
      $forms_avaliable[$identifier] = $content_model->getIngestFormNames();
    }
    $form['indicator'] = array(
      '#type' => 'fieldset',
      '#title' => t('Ingest digital object into collection_pid !collection_label Step #1',
          array('collection_pid' => $this->collectionPid, '!collection_label' => $this->collectionLabel))
    );
    $form['indicator']['models'] = array(// Content models available
      '#type' => 'select',
      '#title' => t('Content models available'),
      '#options' => $models_for_form,
      '#description' => t('Content models define datastream composition, relationships between this and other ' .
          'content models, and the mandatory behaviors associated with each digital object.<br /> Additional ' .
          'information may be found ' .
          '<a href="https://wiki.duraspace.org/display/FEDORACREATE/Content+Models+Overview">here.</a> ')
    );
    $form['indicator']['forms'] = array(// Forms available
      '#type' => 'select',
      '#title' => t('Forms available'),
      '#options' => $forms_avaliable,
      '#description' => t('The form to use for ingesting the digital object.')
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next')
    );
    return $form;
  }

  /**
   *
   */
  private function getFormPageTwo() {
    $form = &$this->createIngestFormClass();
    return $form->createForm();
  }

  /**
   * Validates the ingest form based on the forms current page number.
   *
   * @param array
   *   Drupal form to be validated.
   */
  public function validateForm(&$form) {
    $this->page == 1 ?
            $this->validateFormOne($form) :
            $this->validateFormTwo($form);
  }

  /**
   * Changes the form to page to and rebuilds.
   */
  private function validateFormOne(&$form) {
    $form_values = &$this->formState['values'];
    $this->storage['content_model_pid'] = ContentModel::getPidFromIdentifier($form_values['models']);
    $this->storage['content_model_dsid'] = ContentModel::getDSIDFromIdentifier($form_values['models']);
    $this->storage['form_name'] = $form_values['forms'];
    $this->page++;
    $this->formState['rebuild'] = TRUE;
  }

  /**
   * Validate the second page of the ingest form. This should really be refactored out such that a the user
   * of this class can specifiy thier own validator.
   *
   * @param array $form
   *   Drupal form to be validated.
   */
  private function validateFormTwo(&$form) {
    $form = &$this->createIngestFormClass();
    return $form->validateForm($form);
  }

  /**
   *
   * @param array
   *  $form
   */
  public function submitForm(&$form) {
    $form = &$this->createIngestFormClass();
    return $form->submitForm($form);
  }

  private function &createIngestFormClass() {
    $content_model = ContentModel::loadFromModel($this->contentModelPid, $this->contentModelDsid);
    $form_element = $content_model->getForm($this->formName);
    $module = $form_element->getAttribute('ingest_module');
    $filename = $form_element->getAttribute('ingest_file');
    $class = $form_element->getAttribute('ingest_class');
    $file = './' . drupal_get_path('module', $module) . "/$filename";
    if (is_file($file)) {
      require_once $file;
    }
    else {
      return FALSE;
    }
    $form = new $class($this->formState);
    return $form;
  }

}