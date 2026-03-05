<?php

/**
 * @file
 * Generate header files for all TXT files in the txt/ directory.
 */

$typedefs = [];

function cid(string $name, string $altname, bool $pascalcase = TRUE) : string {
  $newname = trim(preg_replace('/[^a-z0-9_ ]+/i', ' ', $name), ' ');
  $newname = $pascalcase && str_contains($newname, ' ') ? ucwords($newname) : $newname;
  $newname = str_replace(' ', '', $newname);

  if (!$newname) {
    $newname = "$altname";
  }

  if (is_numeric($newname[0]) || in_array($newname, [
    // C++ reserved keywords.
    'alignas',
    'alignof',
    'and',
    'and_eq',
    'asm',
    'auto',
    'bitand',
    'bitor',
    'bool',
    'break',
    'case',
    'catch',
    'char',
    'char8_t',
    'char16_t',
    'char32_t',
    'class',
    'compl',
    'concept',
    'const',
    'const_cast',
    'consteval',
    'constexpr',
    'constinit',
    'continue',
    'co_await',
    'co_return',
    'co_yield',
    'decltype',
    'default',
    'delete',
    'do',
    'double',
    'dynamic_cast',
    'else',
    'enum',
    'explicit',
    'export',
    'extern',
    'false',
    'float',
    'for',
    'friend',
    'goto',
    'if',
    'inline',
    'int',
    'long',
    'mutable',
    'namespace',
    'new',
    'noexcept',
    'not',
    'not_eq',
    'nullptr',
    'operator',
    'or',
    'or_eq',
    'private',
    'protected',
    'public',
    'register',
    'reinterpret_cast',
    'requires',
    'return',
    'short',
    'signed',
    'sizeof',
    'static',
    'static_assert',
    'static_cast',
    'struct',
    'switch',
    'template',
    'this',
    'thread_local',
    'throw',
    'true',
    'try',
    'typedef',
    'typeid',
    'typename',
    'union',
    'unsigned',
    'using',
    'virtual',
    'void',
    'volatile',
    'wchar_t',
    'while',
    'xor_eq',
  ])) {
    $newname = "_$newname";
  }

  return $newname;
}

foreach ([
  ['ROTW', 'txtrotw', 'txt/*.txt'],
  ['BASE', 'txtbase', 'txt/base/*.txt'],
] as [$namespace, $filebase, $path]) {
  foreach (glob(__DIR__ . '/' . $path) as $i => $file) {
    $name = cid(basename($file, '.txt'), "txt_$i");

    $handle = fopen($file, 'r');
    $header = fgets($handle);
    $headers = explode("\t", trim($header));

    foreach ($headers as $i => $header) {
      $headers[$i] = $header;
    }

    $headerTypes = array_map(fn () => 'void', $headers);

    while (($line = fgets($handle)) !== false) {
      $values = explode("\t", trim($line));

      foreach ($values as $i => $value) {
        if (isset($headerTypes[$i])) {
          if ($value !== '') {
            if (is_numeric($value)) {
              if ($headerTypes[$i] === 'void') {
                $headerTypes[$i] = 'int32_t';
              }
            }
            else {
              $headerTypes[$i] = 'std::string';
            }
          }
        }
      }
    }

    fclose($handle);

    $typedefs[$name] = array_combine($headers, $headerTypes);
  }

  $output = <<<CPP
    #include <cstdint>
    #include <string>
    #include <vector>

    namespace D2::$namespace {
    CPP;

  $output .= "\n\n";

  foreach ($typedefs as $name => $fields) {
    $name = 't_' . $name;
    $output .= "  struct $name {\n";

    $i = 0;
    foreach ($fields as $field => $type) {
      if ($type === 'void') {
        $output .= "    // Unused column: $field\n";
      }
      else {
        $field = cid($field, "field_$field", FALSE);
        $default_value = $type === 'std::string' ? '""' : '0';
        $output .= "    $type $field = $default_value;\t// Column $i\n";
      }
      $i++;
    }

    $output .= "\n    size_t read(const char* line);\n";
    $output .= "    static std::vector<$name> readfile(std::string filename);\n";

    $output .= "  };\n\n";
  }

  $output .= "}\n";

  file_put_contents(__DIR__ . '/' . $filebase . '.h', $output);

  $output = <<<CPP
    #include "$filebase.h"
    #include <string>
    #include <vector>
    #include <fstream>
    #include <sstream>

    namespace D2::$namespace {
    CPP;

  $output .= "\n\n";

  foreach ($typedefs as $name => $fields) {
    $name = 't_' . $name;

    $output .= <<<CPP
      size_t $name::read(const char* line) {
        std::vector<std::string> values;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, '\\t')) {
          values.push_back(value);
        }
    CPP;
    $output .= "\n\n";

    $i = 0;
    foreach ($fields as $field => $type) {
      if ($type !== 'void') {
        $field = cid($field, "field_$field", FALSE);
        $output .= "    if (values.size() > $i) { ";
        if ($type === 'int32_t') {
          $output .= "$field = values[$i][0] == '\\0' ? 0 : std::stoi(values[$i]);";
        }
        else {
          $output .= "$field = values[$i];";
        }
        $output .= " }\n";
      }
      $i++;
    }

    $output .= "\n    return values.size();\n  }\n\n";

    $output .= <<<CPP
      std::vector<$name> $name::readfile(std::string filename) {
        std::vector<$name> ret;
        std::ifstream file(filename);
  
        if (!file.is_open()) {
          return ret; // Return empty vector if file cannot be opened
        }
  
        std::string line;
        if (!std::getline(file, line)) {
          return ret; // Return empty vector if file is empty
        }
        while (std::getline(file, line)) {
          $name v;
          v.read(line.c_str());
          ret.push_back(v);
        }
  
        return ret;
      }
    CPP;

    $output .= "\n\n";
  }

  $output .= "}\n";

  file_put_contents(__DIR__ . '/' . $filebase . '.cpp', $output);
}
