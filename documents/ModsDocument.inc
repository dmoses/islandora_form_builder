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
class ModsDocument extends XmlDatastreamFormDocument {
  const mods = 'http://www.loc.gov/mods/v3';
  const xlink = 'http://www.w3.org/1999/xlink';
  const xsi = 'http://www.w3.org/2001/XMLSchema-instance';
  const xmlns = 'http://www.w3.org/2000/xmlns/';

  /**
   * Creates a ModsDocument, most processing is done in the parent class.
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
    return array('mods', self::mods);
  }

  /**
   * Gets a map of the required namespaces.
   *
   * @return array
   *   A map of required namepaces where the key's are the prefixs and the values are URI's
   */
  public static function getRequiredNamespaces() {
    return array(
      'mods' => self::mods,
      'xlink' => self::xlink,
      'xsi' => self::xsi,
    );
  }

  /**
   * Create the root element, called from parent.
   */
  protected function createRootElement() {
    $root = $this->createElementNS(self::mods, 'mods');
    $this->setRootAttributes($root);
    return $root;
  }

  /**
   * Correct/Add required root attributes.
   */
  private function setRootAttributes($root) {
    $root->setAttributeNS(self::xmlns, 'xmlns', self::mods);
    $root->setAttributeNS(self::xmlns, 'xmlns:mods', self::mods);
    $root->setAttributeNS(self::xmlns, 'xmlns:xlink', self::xlink);
    $root->setAttributeNS(self::xmlns, 'xmlns:xsi', self::xsi);
    $this->setCorrectVersion($root);
  }

  /**
   * Set the mods version attribute to its proper value.
   *
   * @param DOMElement $root
   *   Root element.
   */
  private function setCorrectVersion($root) {
    $version = $root->getAttribute('version');
    switch ($version) {
      case '3.0':
        $root->setAttribute('version', '3.0');
        $root->setAttributeNS(self::xsi, 'xsi:schemaLocation', self::mods . " http://www.loc.gov/standards/mods/v3/mods-3-0.xsd");
        break;
      case '3.1':
        $root->setAttribute('version', '3.1');
        $root->setAttributeNS(self::xsi, 'xsi:schemaLocation', self::mods . " http://www.loc.gov/standards/mods/v3/mods-3-1.xsd");
        break;
      case '3.2':
        $root->setAttribute('version', '3.2');
        $root->setAttributeNS(self::xsi, 'xsi:schemaLocation', self::mods . " http://www.loc.gov/standards/mods/v3/mods-3-2.xsd");
        break;
      case '3.3':
        $root->setAttribute('version', '3.3');
        $root->setAttributeNS(self::xsi, 'xsi:schemaLocation', self::mods . " http://www.loc.gov/standards/mods/v3/mods-3-3.xsd");
        break;
      default:
        $root->setAttribute('version', '3.4');
        $root->setAttributeNS(self::xsi,'xsi:schemaLocation', self::mods . " http://www.loc.gov/standards/mods/v3/mods-3-4.xsd");
        break;
    }
  }

}