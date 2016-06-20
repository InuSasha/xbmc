/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DirectoryNodeHomeTvShows.h"
#include "FileItem.h"
#include "QueryParams.h"
#include "video/VideoDatabase.h"

using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeHomeTvShows::CDirectoryNodeHomeTvShows(const std::string& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_HOME_TVSHOWS, strName, pParent)
{

}

bool CDirectoryNodeHomeTvShows::GetContent(CFileItemList& items) const
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);

  // select watching tvshows
  CDatabase::Filter filter;
  filter.fields  = "e.*";
  filter.join    = videodatabase.PrepareSQL(
    "e "
    "right join tvshow_view t on (e.idShow = t.idShow) "
    "right join ("
      "SELECT idShow, MIN(IF(c%02d = -1, c%02d, c%02d) * 4096 + IF(c%02d = -1, c%02d, c%02d-1+c%02d/10000)) AS sort "
      "FROM episode_view "
      "WHERE playCount is NULL "
      "GROUP BY idShow"
    ") s on (e.idShow = s.idShow)",
    VIDEODB_ID_EPISODE_SORTSEASON,  VIDEODB_ID_EPISODE_SEASON,  VIDEODB_ID_EPISODE_SORTSEASON,
    VIDEODB_ID_EPISODE_SORTEPISODE, VIDEODB_ID_EPISODE_EPISODE, VIDEODB_ID_EPISODE_SORTEPISODE, VIDEODB_ID_EPISODE_EPISODE
  );
  filter.where   = videodatabase.PrepareSQL(
    "t.watchedCount != 0 "
    "AND s.sort = IF(e.c%02d = -1, e.c%02d, e.c%02d) * 4096 + IF(e.c%02d = -1, e.c%02d, e.c%02d-1+e.c%02d/10000)",
    VIDEODB_ID_EPISODE_SORTSEASON,  VIDEODB_ID_EPISODE_SEASON,  VIDEODB_ID_EPISODE_SORTSEASON,
    VIDEODB_ID_EPISODE_SORTEPISODE, VIDEODB_ID_EPISODE_EPISODE, VIDEODB_ID_EPISODE_SORTEPISODE, VIDEODB_ID_EPISODE_EPISODE
  );

  SortDescription sort;
  sort.sortBy    = SortByTvShowTitle;
  sort.sortOrder = SortOrderAscending;

  bool bSuccess=videodatabase.GetEpisodesByWhere("videodb://tvshows/titles/", filter, items, true, sort);
  if ( ! bSuccess )
  {
    videodatabase.Close();
    return false;
  }

  // select recentlyadded tvshows
  filter.where   = videodatabase.PrepareSQL(
      "t.watchedCount = 0 "
      "AND s.sort = IF(e.c%02d = -1, e.c%02d, e.c%02d) * 4096 + IF(e.c%02d = -1, e.c%02d, e.c%02d-1+e.c%02d/10000)",
      VIDEODB_ID_EPISODE_SORTSEASON,  VIDEODB_ID_EPISODE_SEASON,  VIDEODB_ID_EPISODE_SORTSEASON,
      VIDEODB_ID_EPISODE_SORTEPISODE, VIDEODB_ID_EPISODE_EPISODE, VIDEODB_ID_EPISODE_SORTEPISODE, VIDEODB_ID_EPISODE_EPISODE
  );
  filter.order   = videodatabase.PrepareSQL( "t.dateAdded desc");
  sort.sortBy    = SortByNone;

  CFileItemList recentlyadded;
  bSuccess=videodatabase.GetEpisodesByWhere("videodb://tvshows/titles/", filter, recentlyadded, true, sort);
  if ( ! bSuccess )
  {
    videodatabase.Close();
    return false;
  }

  items.Append( recentlyadded);

  videodatabase.Close();

  return true;
}
