<?php

// $Id$

/**
 * @file
 *
 */
module_load_include('inc', 'islandora_form_builder', 'XmlDatastream');

/**
 *
 */
class PBCoreDocument extends XmlDatastreamFormDocument {
  const pbcore = 'http://www.pbcore.org/PBCore/PBCoreNamespace.html';
  const xsi = 'http://www.w3.org/2001/XMLSchema-instance';
  const xmlns = 'http://www.w3.org/2000/xmlns/';

  /**
   * Creates a PBCoreDocument, most processing is done in the parent class.
   *
   * @param array $form
   * @param array $form_state
   */
  public function __construct(&$form, &$form_state) {
    parent::__construct($form, $form_state, self::getRequiredNamespaces());
    $this->setCorrectVersion($this->documentElement);
  }

  /**
   * Gets the default namespace need for xpath to function.
   * 
   * @return array
   */
  public static function getDefaultNamespace() {
    return array('pbcore', self::pbcore);
  }

  /**
   * Gets a map of the required namespaces.
   *
   * @return array
   *   A map of required namepaces where the key's are the prefixs and the values are URI's
   */
  public static function getRequiredNamespaces() {
    return array(
      'pbcore' => self::pbcore,
      'xsi' => self::xsi,
    );
  }

  /**
   * Create the root element, called from parent.
   */
  protected function createRootElement() {
    $root = $this->createElementNS(self::pbcore, 'pbcore');
    $this->setRootAttributes($root);
    return $root;
  }

  /**
   * Correct/Add required root attributes.
   */
  private function setRootAttributes($root) {
    $root->setAttributeNS(self::xmlns, 'xmlns', self::pbcore);
    $root->setAttributeNS(self::xmlns, 'xmlns:pbcore', self::pbcore);
    $root->setAttributeNS(self::xmlns, 'xmlns:xsi', self::xsi);
    $this->setCorrectVersion($root);
  }

  /**
   * Set the pbcore version attribute to its proper value.
   *
   * @param DOMElement $root
   *   Root element.
   */
  private function setCorrectVersion($root) {
    $version = $root->getAttribute('version');
    switch ($version) {
      case '2.0':
        $root->setAttribute('version', '2.0');
        $root->setAttributeNS(self::xsi, 'xsi:schemaLocation', self::pbcore . " http://pbcore.org/xsd/pbcore-2.0.xsd");
        break;
      default:
        $root->setAttribute('version', '2.0');
        $root->setAttributeNS(self::xsi, 'xsi:schemaLocation', self::pbcore . " http://pbcore.org/xsd/pbcore-2.0.xsd");
        break;
    }
  }

}

