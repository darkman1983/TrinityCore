DELETE FROM `achievement_criteria_data` WHERE `criteria_id` IN (10223,10240,10241,10229,10238,10239) AND `type`=11;
INSERT INTO `achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES
(10223,11,0,0, 'achievement_staying_buffed_all_winter'),
(10229,11,0,0, 'achievement_staying_buffed_all_winter'),
(10238,11,0,0, 'achievement_staying_buffed_all_winter'),
(10239,11,0,0, 'achievement_staying_buffed_all_winter'),
(10240,11,0,0, 'achievement_staying_buffed_all_winter'),
(10241,11,0,0, 'achievement_staying_buffed_all_winter');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (62821,62807);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(62821, 'spell_hodir_toasty_fire'),
(62807, 'spell_hodir_starlight');
