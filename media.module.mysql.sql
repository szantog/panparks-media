# phpMyAdmin SQL Dump
# version 2.5.7-pl1
# http://www.phpmyadmin.net
#
# Host: localhost
# Generation Time: Jan 28, 2005 at 11:09 PM
# Server version: 4.0.20
# PHP Version: 4.3.4
# 
# Database : `fxy`
# 

# --------------------------------------------------------

#
# Table structure for table `av_metadata`
#

CREATE TABLE `audio_metadata` (
  `fid` int(10) unsigned NOT NULL default '0',
  `fileformat` varchar(64) default NULL,
  `audio_dataformat` varchar(64) default NULL,
  `audio_channels` int(2) unsigned default NULL,
  `audio_sample_rate` int(10) unsigned default NULL,
  `audio_bitrate` int(10) unsigned default NULL,
  `audio_channelmode` varchar(64) default NULL,
  `audio_compression_ratio` varchar(64) default NULL,
  `audio_codec` varchar(64) default NULL,
  `audio_encoder` varchar(64) default NULL,
  `id3_title` varchar(128) default NULL,
  `id3_artist` varchar(128) default NULL,
  `id3_album` varchar(128) default NULL,
  `id3_comment` varchar(128) default NULL,
  `id3_track` varchar(128) default NULL,
  `id3_genre` varchar(128) default NULL,
  `id3_year` varchar(128) default NULL,
  `encoding` varchar(128) default NULL,
  `playtime_seconds` varchar(128) default NULL,
  `playtime_string` varchar(128) default NULL,
  PRIMARY KEY  (`fid`)
) TYPE=MyISAM;


CREATE TABLE `video_metadata` (
  `fid` int(10) unsigned NOT NULL default '0',
  `fileformat` varchar(64) default NULL,
  `video_bitrate` int(10) unsigned default NULL,
  `video_resolution_x` int(10) unsigned default NULL,
  `video_resolution_y` int(10) unsigned default NULL,
  `video_frame_rate` int(10) unsigned default NULL,
  `video_pixel_aspect_ratio` int(2) unsigned default NULL,
  `video_bits_per_sample` int(10) unsigned default NULL,
  `video_codec` varchar(64) default NULL,
  `video_compression_ratio` varchar(64) default NULL,
  `encoding` varchar(128) default NULL,
  `playtime_seconds` varchar(128) default NULL,
  `playtime_string` varchar(128) default NULL,
  PRIMARY KEY  (`fid`)
) TYPE=MyISAM;

#
# Table structure for table `av_playlist`
#

CREATE TABLE `av_playlist` (
  `nid` int(10) NOT NULL default '0',
  `fid` int(10) NOT NULL default '0',
  `next_fid` int(10) NOT NULL default '0',
  KEY `nid` (`nid`)
) TYPE=MyISAM;

