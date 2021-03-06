-- add missing Quest Mark & Quest Accept Conditions
/*
SELECT *FROM `conditions` WHERE `SourceTypeOrReferenceId`=20 AND `SourceEntry` NOT IN (SELECT `SourceEntry` FROM `conditions` WHERE `SourceTypeOrReferenceId`=19);
SELECT *FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` NOT IN (SELECT `SourceEntry` FROM `conditions` WHERE `SourceTypeOrReferenceId`=20);
*/
INSERT IGNORE INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,13044,0,8,13008,0,0,0,'',''),
(19,0,13044,0,8,13039,0,0,0,'',''),
(19,0,13044,0,8,13040,0,0,0,'',''),
(19,0,13068,0,8,13139,0,0,0,'',''),
(19,0,13086,0,8,13070,0,0,0,'',''),
(19,0,13104,0,8,13086,0,0,0,'',''),
(19,0,13104,1,15,11,0,0,0,'',''),
(19,0,13104,1,15,9,0,0,0,'',''),
(19,0,13104,1,15,8,0,0,0,'',''),
(19,0,13104,1,15,7,0,0,0,'',''),
(19,0,13104,1,15,5,0,0,0,'',''),
(19,0,13104,1,15,4,0,0,0,'',''),
(19,0,13104,1,15,3,0,0,0,'',''),
(19,0,13104,1,15,2,0,0,0,'',''),
(19,0,13104,1,15,1,0,0,0,'',''),
(19,0,13105,0,15,6,0,0,0,'',''),
(19,0,13105,0,8,13086,0,0,0,'',''),
(19,0,13120,1,8,13104,0,0,0,'',''),
(19,0,13120,1,8,13105,0,0,0,'',''),
(19,0,13224,0,8,13157,0,0,0,'',''),
(19,0,13225,0,8,13157,0,0,0,'',''),
(19,0,13118,1,8,13122,0,0,0,'',''),
(19,0,13122,1,8,13104,0,0,0,'',''),
(19,0,13122,1,8,13105,0,0,0,'',''),
(19,0,13125,0,8,13110,0,0,0,'',''),
(19,0,13125,0,8,13118,0,0,0,'',''),
(19,0,13130,0,8,13110,0,0,0,'',''),
(19,0,13130,0,8,13118,0,0,0,'',''),
(19,0,13135,0,8,13110,0,0,0,'',''),
(19,0,13135,0,8,13118,0,0,0,'',''),
(19,0,13139,0,8,13125,0,0,0,'',''),
(19,0,13139,0,8,13130,0,0,0,'',''),
(19,0,13139,0,8,13135,0,0,0,'',''),
(20,0,8870,0,4,1537,0,0,0,'','The Lunar Festival: Ironforge'),
(20,0,8871,0,4,1519,0,0,0,'','The Lunar Festival: Stormwind'),
(20,0,8872,0,4,1657,0,0,0,'','The Lunar Festival: Darnassus'),
(20,0,8873,0,4,1637,0,0,0,'','The Lunar Festival: Orgrimmar'),
(20,0,8874,0,4,1497,0,0,0,'','The Lunar Festival: Undercity'),
(20,0,8875,0,4,1638,0,0,0,'','The Lunar Festival: Thunder Bluff'),
(20,0,11360,0,23,87,0,0,0,'',''),
(20,0,11361,0,23,362,0,0,0,'',''),
(20,0,11439,0,23,131,0,0,0,'',''),
(20,0,11440,0,23,3576,0,0,0,'',''),
(20,0,11449,0,23,159,0,0,0,'',''),
(20,0,11450,0,23,3665,0,0,0,'','');