CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT);

INSERT OR IGNORE INTO TestTable (platform, vendor)
VALUES
('iOS','Apple'),
('Android','Google'),
('HarmonyOS','Huawei'),
('Windows','MicroSoft'),
('Ubuntu','Canonical'),
('PlayStation','Sony'),
('Switch','Nintendo'),
('XBox','Microsoft');
