const fs = require('fs');

let totals = {};

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

fs.readdir(__dirname + '/simulations/', (err, files) => {
  files.forEach(async function (file) {
    if (file.endsWith('.json')) {
      let data = null;

      for (let c = 0; c < 3 && !data; c++) {
        try {
          data = JSON.parse(fs.readFileSync(__dirname + '/simulations/' + file));
        }
        catch (e) {
          await sleep(50);
        }
      }

      let tc = data.tc + ' [' + data.playermod + ']';

      totals[tc] = totals[tc] || {
        tc: data.tc,
        runs: 0,
        picks: 0,
        avgpicks: 0,
        playermod: data.playermod,
        drops: {},
      };

      let total = totals[tc];

      total.runs += data.runs;
      total.picks += data.picks;
      total.avgpicks = total.picks / total.runs;

      Object.keys(data.drops).forEach(key => {
        if (!total.drops[key]) {
          total.drops[key] = 0;
        }

        total.drops[key] += data.drops[key];
      });
    }
  });

  for (const tc in totals) {
    const total = totals[tc];
    total.drops = Object.fromEntries(Object.entries(total.drops).map(([key, value]) => [key, value / total.runs]).sort((a, b) => b[1] - a[1]));
    fs.writeFileSync(__dirname + '/simulation-totals/' + tc + '.json', JSON.stringify(total, null, '  '));
  }
});
