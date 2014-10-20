<?php
/**
 * @file
 * @author David Walters <david.walters@open.edu.au>
 * Date: 3/06/2014
 * Time: 12:27 PM
 */

namespace templates\test;
use DOMDocument;

define('DRUPAL_ROOT', __DIR__ . '/../../../../../../');
require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_override_server_variables();
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

abstract class TemplateTestBase extends \PHPUnit_Framework_TestCase {

  /**
   * Setup the test, clear any libxml errors.
   */
  public function setUp() {
    libxml_clear_errors();
  }

  /**
   * Validate XML fragment.
   *
   * @param string $contents
   *   The XML string to validate.
   */
  public function AssertValidXml($contents) {
    $a = new DOMDocument();
    // Allow us to process errors rather than just sending them to me.
    libxml_use_internal_errors(TRUE);
    $loaded = $a->loadXML($contents);
    $errors = libxml_get_errors();
    $error_message = $this->ConstructErrorMessage('XML Errors found: ', $contents, $errors);
    $this->AssertTrue($loaded, "Failed to load XML Document\n\n$contents");
    // Ensure we are error free in the template.
    $this->assertCount(0, $errors, $error_message);
  }

  /**
   * Validate XML templates - used for HTML5 validation.
   *
   * (until PHP/libXml2 catches up, or we take a punt on html5lib)
   *
   * @param string $filename
   *   The name of the file to validate.
   * @param array $vars
   *   The variables to export to global namespace so the template will render.
   * @param bool   $prepend_declaration
   *   Prepend the XML fragment with a <?xml?> declaration.
   */
  public function AssertTemplateValidXml($filename, $vars = NULL, $prepend_declaration = TRUE) {
    $contents = $this->IncludeFile($filename, $vars);
    if ($prepend_declaration) {
      $contents = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" . $contents;
    }
    $this->AssertValidXml($contents);
  }

  /**
   * Validate an HTML string.
   *
   * @param string $html_string
   *   The string to validate.
   */
  public function AssertValidHtml($html_string) {
    $a = new DOMDocument();
    // Allow us to process errors rather than just sending them to me.
    libxml_use_internal_errors(TRUE);
    $loaded = $a->loadHTML($html_string);
    $errors = libxml_get_errors();
    $error_message = $this->ConstructErrorMessage("HTML Errors Found:", $html_string, $errors);
    $this->AssertTrue($loaded, "Failed to load HTML document:\n\n $html_string");
    // Ensure we are error free in the template.
    $this->assertCount(0, $errors, $error_message);
  }

  /**
   * Validate HTML template.
   *
   * @param string $filename
   *   The name of the template file to validate.
   * @param array $vars
   *   The variables to export to global namespace so the template will render.
   * @param bool $wrap_tags
   *   Whether or not to wrap the HTML fragment in <html> tags.
   */
  public function AssertTemplateValidHtml($filename, $vars = NULL, $wrap_tags = TRUE) {
    $contents = $this->IncludeFile($filename, $vars);
    if ($wrap_tags) {
      $contents = "<html>\n" . $contents . "\n</html>";
    }
    $this->AssertValidHtml($contents);
  }

  /**
   * Format a string with a gutter of line numbers.
   *
   * @param string $string
   *   The string to format.
   *
   * @param array $errors
   *   The list of error objects returned from libxml_get_errors()
   *
   * @return string
   *   The formatted string containing a gutter of line numbers.
   */
  public function FormatLineNumbers($string, $errors = NULL) {
    $error_lines = array();
    if ($errors != NULL) {
      foreach ($errors as $error) {
        $error_lines[] = (int)$error->line;
      }
    }
    $lines = preg_split('/$\R?^/m', $string);
    $newlines = "";
    $line_number = 1;
    foreach ($lines as $line) {
      $gutter = sprintf("%1$04d", $line_number);
      $marker = in_array($line_number, $error_lines) ? "*" : "-";
      $newlines .= "$gutter $marker  $line\n";
      $line_number++;
    }
    return $newlines;
  }

  /**
   * Construct a meaningful error message.
   *
   * @param string $title
   *   The title message to display.
   * @param string $contents
   *   The validated content.
   * @param array $errors
   *   The set of libXml errors.
   *
   * @return string
   *   The formatted error message.
   */
  private function ConstructErrorMessage($title, $contents, $errors) {
    $error_message = $title
      . var_export($errors, TRUE)
      . "\n\n\nDocument was:\n\n"
      . $this->FormatLineNumbers($contents, $errors);
    return $error_message;
  }

  /**
   * Include the file to validate.
   *
   * @param string $filename
   *   The name of the file to include
   * @param array $vars
   *   An array of vars to extract to the global namespace.
   *
   * @return string
   *   The processed file.
   */
  protected function IncludeFile($filename, $vars) {
    ob_start();
    extract($vars);
    include $filename;
    $contents = ob_get_contents();
    ob_end_clean();
    return $contents;
  }


}