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

#include "DirectoryNodeHomeMovies.h"
#include "QueryParams.h"
#include "video/VideoDatabase.h"

using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeHomeMovies::CDirectoryNodeHomeMovies(const std::string& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_HOME_MOVIES, strName, pParent)
{

}

bool CDirectoryNodeHomeMovies::GetContent(CFileItemList& items) const
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);

  CDatabase::Filter filter;
  filter.fields  = "m.*";
  filter.join    = "m left join (select idSet, min(premiered) as premiered from movie_view where playCount is NULL and idSet is not null group by idSet) p on (m.idSet = p.idSet)";
  filter.where   = "m.playCount IS NULL AND (m.idSet is NULL or m.premiered = p.premiered)";
  filter.group   = "coalesce(m.idSet,CONCAT('a',m.idMovie))";

  SortDescription sort;
  sort.sortBy    = SortByDateAdded;
  sort.sortOrder = SortOrderDescending;

  bool bSuccess=videodatabase.GetMoviesByWhere("videodb://movies/", filter, items, sort);

  videodatabase.Close();

  return bSuccess;
}
