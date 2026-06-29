-- Database schemas

START TRANSACTION;

-- URL tables
CREATE TABLE urls (
  id            UUID      PRIMARY KEY    ,
  url           VARCHAR   NOT NULL UNIQUE,
  discovered_at TIMESTAMP NOT NULL       , 
  last_crawled  TIMESTAMP NOT NULL
);

CREATE TABLE url_contents (
  url_id          UUID  PRIMARY KEY REFERENCES urls(id), 
  compressed_html BYTEA NOT NULL
);

CREATE TABLE url_metadatas (
  url_id         UUID   PRIMARY KEY REFERENCES urls(id),
  content_hash   BYTEA  NOT NULL                     , 
  simhash        BIGINT NOT NULL                     ,
  content_length BIGINT NOT NULL
);

CREATE TABLE url_outlinks (
  src_url  UUID REFERENCES urls(id) NOT NULL, 
  dest_url UUID REFERENCES urls(id) NOT NULL
);

-- Politeness 
CREATE TYPE ROBOTS_TXT_CONSTRAINT AS ENUM ('Allow', 'Disallow', 'CrawlDelay');
CREATE TABLE robots_txt (
  id            UUID                  PRIMARY KEY,
  domain        VARCHAR               NOT NULL   ,
  category      ROBOTS_TXT_CONSTRAINT NOT NULL   ,
  relative_path VARCHAR,
  crawl_delay   BIGINT
);

COMMIT;
