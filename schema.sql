-- MySQL Database Schema for Tulospalvelu
-- This schema supports both relay races (viesti) and individual competitions (hk)
-- Copyright (C) 2015 Pekka Pirila
-- License: GNU General Public License v3.0

-- =============================================
-- RELAY RACE TABLES (ViestiWin)
-- =============================================

-- Table for relay team entries
CREATE TABLE IF NOT EXISTS vosanotot (
    kilpailu VARCHAR(50) NOT NULL,
    tietue INT NOT NULL,
    id INT,
    sarja VARCHAR(50),
    seura VARCHAR(60),
    joukkue INT,
    joukkid INT,
    joukkuenimi VARCHAR(32),
    maa VARCHAR(4),
    piiri INT,
    ilmlista INT,
    piste1 INT,
    piste2 INT,
    piste3 INT,
    era INT,
    PRIMARY KEY (kilpailu, tietue),
    INDEX (kilpailu)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Table for relay leg information
CREATE TABLE IF NOT EXISTS osuudet (
    kilpailu VARCHAR(50) NOT NULL,
    tietue INT NOT NULL,
    osuus INT NOT NULL,
    lisno INT,
    sukunimi VARCHAR(60),
    etunimi VARCHAR(60),
    arvo VARCHAR(30),
    seura VARCHAR(40),
    badge INT,
    badge2 INT,
    laina CHAR(1),
    laina2 CHAR(1),
    rata VARCHAR(40),
    selitys VARCHAR(40),
    tlahto BIGINT,
    tlahtotime VARCHAR(20),
    lahtolaji INT,
    keskhyl CHAR(1),
    ampsakot VARCHAR(20),
    sakko INT,
    gps CHAR(1),
    ossija INT,
    PRIMARY KEY (kilpailu, tietue, osuus),
    INDEX (kilpailu, tietue),
    INDEX (kilpailu)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Table for relay results and split times
CREATE TABLE IF NOT EXISTS vtulos (
    kilpailu VARCHAR(50) NOT NULL,
    tietue INT NOT NULL,
    osuus INT NOT NULL,
    piste INT NOT NULL,
    aika BIGINT,
    aikatime VARCHAR(20),
    sija INT,
    PRIMARY KEY (kilpailu, tietue, osuus, piste),
    INDEX (kilpailu, tietue),
    INDEX (kilpailu)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- =============================================
-- INDIVIDUAL COMPETITION TABLES (cbHk)
-- =============================================

-- Table for individual competitor entries
CREATE TABLE IF NOT EXISTS osanotot (
    kilpailu VARCHAR(50) NOT NULL,
    tietue INT NOT NULL,
    id INT,
    lisno INT,
    intid INT,
    wrkoodi VARCHAR(20),
    sukunimi VARCHAR(60),
    etunimi VARCHAR(60),
    arvo VARCHAR(30),
    seura VARCHAR(60),
    seuralyh VARCHAR(20),
    yhdistys VARCHAR(60),
    joukkue VARCHAR(60),
    maa VARCHAR(4),
    piiri INT,
    ilmlista INT,
    piste1 INT,
    piste2 INT,
    piste3 INT,
    sarja VARCHAR(50),
    sukup CHAR(1),
    ikasarja INT,
    synt INT,
    alisarja INT,
    arvryhma INT,
    PRIMARY KEY (kilpailu, tietue),
    INDEX (kilpailu),
    INDEX (sarja),
    INDEX (seura)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Table for individual competitor race day data
CREATE TABLE IF NOT EXISTS osottopv (
    kilpailu VARCHAR(50) NOT NULL,
    tietue INT NOT NULL,
    vaihe INT NOT NULL,
    sarja VARCHAR(50),
    era INT,
    bib INT,
    badge INT,
    badge2 INT,
    laina CHAR(1),
    laina2 CHAR(1),
    rata VARCHAR(40),
    selitys VARCHAR(40),
    pvpiste1 INT,
    pvpiste2 INT,
    tlahto BIGINT,
    tlahtotime VARCHAR(20),
    keskhyl CHAR(1),
    ampsakot VARCHAR(20),
    tasoitus INT,
    sakko INT,
    gps CHAR(1),
    qual CHAR(1),
    yhtaika BIGINT,
    yhtaikatime VARCHAR(20),
    ysija INT,
    PRIMARY KEY (kilpailu, tietue, vaihe),
    INDEX (kilpailu, tietue),
    INDEX (kilpailu, vaihe),
    INDEX (kilpailu)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Table for individual results and split times
CREATE TABLE IF NOT EXISTS tulos (
    kilpailu VARCHAR(50) NOT NULL,
    tietue INT NOT NULL,
    vaihe INT NOT NULL,
    piste INT NOT NULL,
    aika BIGINT,
    aikatime VARCHAR(20),
    sija INT,
    PRIMARY KEY (kilpailu, tietue, vaihe, piste),
    INDEX (kilpailu, tietue),
    INDEX (kilpailu, vaihe),
    INDEX (kilpailu)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- =============================================
-- COMMON INDEXES FOR PERFORMANCE
-- =============================================

-- Create additional indexes for frequently queried columns
CREATE INDEX IF NOT EXISTS idx_vosanotot_kilpailu_sarja ON vosanotot(kilpailu, sarja);
CREATE INDEX IF NOT EXISTS idx_osuudet_kilpailu_osuus ON osuudet(kilpailu, osuus);
CREATE INDEX IF NOT EXISTS idx_vtulos_kilpailu_osuus_piste ON vtulos(kilpailu, osuus, piste);

CREATE INDEX IF NOT EXISTS idx_osanotot_kilpailu_sarja ON osanotot(kilpailu, sarja);
CREATE INDEX IF NOT EXISTS idx_osottopv_kilpailu_vaihe ON osottopv(kilpailu, vaihe);
CREATE INDEX IF NOT EXISTS idx_tulos_kilpailu_vaihe_piste ON tulos(kilpailu, vaihe, piste);

-- =============================================
-- DATABASE COMMENTS
-- =============================================

-- Add comments to tables for documentation
ALTER TABLE vosanotot COMMENT 'Relay race team entries';
ALTER TABLE osuudet COMMENT 'Relay leg information';
ALTER TABLE vtulos COMMENT 'Relay race results and split times';

ALTER TABLE osanotot COMMENT 'Individual competitor entries';
ALTER TABLE osottopv COMMENT 'Individual competitor race day data';
ALTER TABLE tulos COMMENT 'Individual race results and split times';

-- =============================================
-- END OF SCHEMA
-- =============================================
