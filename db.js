// db.js
const { Pool } = require('pg');

const pool = new Pool({
  user: 'administrador',
  host: 'localhost',
  database: 'plant_protect',
  password: 'y_p<ALLQ51n7',
  port: 5433,
});

module.exports = pool;
