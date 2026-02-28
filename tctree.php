<?php

declare(strict_types=1);

/**
 * @file
 * Display a TC tree (only TC names and heirarchy).
 */

chdir(realpath(__DIR__));

define('TREASURECLASSEX', json_decode(file_get_contents('json/treasureclassex.json'), TRUE));

if ($argc < 2) {
  echo "Usage: php tctree.php [TC name]\n";
  exit(1);
}

function print_tc_tree($tc_name, $indent = 0) {
  if (!isset(TREASURECLASSEX[$tc_name])) {
    return;
  }

  $tc = TREASURECLASSEX[$tc_name];

  if ($tc['Picks'] > 0) {
    for ($c = 0; $c < 10; $c++) {
      $name = $tc['Item' . $c] ?? '';
      $prob = $tc['Prob' . $c] ?? 0;

      if ($name && $prob > 0) {
        echo str_repeat("\t", $indent) . "$name @ $prob\n";
        print_tc_tree($name, $indent + 1);
      }
    }
  }
  elseif ($tc['Picks'] < 0) {
    $picks = -$tc['Picks'];

    for ($c = 0; $c < 10; $c++) {
      $name = $tc['Item' . $c] ?? '';
      $prob = $tc['Prob' . $c] ?? 0;

      if ($name && $prob > 0) {
        $sub = min($picks, $prob);

        if ($sub < 0) {
          break;
        }

        echo str_repeat("\t", $indent) . "$name @ $prob\n";
        print_tc_tree($name, $indent + 1);
        $picks -= $sub;
      }
    }
  }
}

print($argv[1]);
print_tc_tree($argv[1], 1);
