/*
 *
 *  Copyright (c) 2021
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CCDOWNLOAD_MG_H
#define CCDOWNLOAD_MG_H

#include <QTableWidget>
#include <QStringList>
#include <QPushButton>

#include <memory>

#include "engines.h"
#include "context.hpp"

#include "utility.h"

class downloadFinished
{
public:
	static const engines::engine * static_engine ;

	downloadFinished()
	{
	}
	downloadFinished( const engines::engine& engine,int index ) :
		m_engine( &engine ),
		m_index( index )
	{
		downloadFinished::m_static_engine = &engine ;
	}
	const engines::engine& engine()
	{
		if( m_engine ){

			return *m_engine ;
		}else{
			//Shouldnt get here
			return *m_static_engine ;
		}
	}
	int index()
	{
		return m_index ;
	}
private:
	const engines::engine * m_engine = nullptr ;
	static const engines::engine * m_static_engine ;
	int m_index ;
};

Q_DECLARE_METATYPE( downloadFinished )

class concurrentDownloadManager
{
public:
	concurrentDownloadManager( const Context& ctx,
				   QLineEdit& lineEdit,
				   QTableWidget& table,
				   QPushButton& cancelButton ) :
		m_ctx( ctx ),
		m_lineEdit( lineEdit ),
		m_table( table ),
		m_cancelButton( cancelButton )
	{
		static int s = qRegisterMetaType< downloadFinished >() ;
		Q_UNUSED( s )
	}
	void cancelled()
	{
		m_cancelled = true ;
	}
	template< typename Function >
	void monitorForFinished( downloadFinished f,Function function )
	{
		if( m_cancelled ){

			for( int s = 0 ; s < m_downloadList.size() ; s++ ){

				m_table.item( s,0 )->setText( m_downloadList[ s ] ) ;
			}

			this->uiEnableAll( true ) ;
			m_cancelButton.setEnabled( false ) ;
		}else{
			m_counter++ ;

			if( m_counter == m_table.rowCount() ){

				this->uiEnableAll( true ) ;
				m_cancelButton.setEnabled( false ) ;
			}else{
				if( m_index < m_table.rowCount() ){

					function( f.engine(),m_index ) ;
				}
			}
		}
	}
	template< typename ConcurrentDownload,
		  typename SequentialDownload >
	void download( const engines::engine& engine,
		       bool downloadConcurrently,
		       int maxNumberOfConcurrency,
		       ConcurrentDownload concurrentDownload,
		       SequentialDownload sequentialDownload )
	{
		if( m_table.rowCount() ){

			if( downloadConcurrently ){

				m_counter = 0 ;
				m_cancelled = false ;
				m_index = 0 ;

				this->uiEnableAll( false ) ;
				m_cancelButton.setEnabled( true ) ;
				m_table.setEnabled( true ) ;

				m_downloadList.clear() ;

				if( maxNumberOfConcurrency < m_table.rowCount() ){

					for( int s = 0 ; s < maxNumberOfConcurrency ; s++ ){

						concurrentDownload( engine,s ) ;
					}
				}else{
					for( int s = 0 ; s < m_table.rowCount() ; s++ ){

						concurrentDownload( engine,s ) ;
					}
				}
			}else{
				QStringList urls ;

				for( int s = 0 ; s < m_table.rowCount() ; s++ ){

					urls.append( m_table.item( s,0 )->text() ) ;
				}

				auto options = m_lineEdit.text() ;

				sequentialDownload( engine,options,urls ) ;
			}
		}
	}
	template< typename Options,typename Logger >
	void download( const engines::engine& engine,
		       const QString& url,
		       Options opts,
		       Logger logger )
	{
		m_index++ ;

		utility::args args( m_lineEdit.text() ) ;

		m_downloadList.append( url ) ;

		utility::run( engine,
			      utility::updateOptions( engine,args,{ url } ),
			      args.quality,
			      false,
			      std::move( opts ),
			      std::move( logger ),
			      utility::make_term_conn( &m_cancelButton,&QPushButton::clicked ) ) ;
	}
private:
	void uiEnableAll( bool ) ;
	int m_counter ;
	int m_index ;
	bool m_cancelled ;
	const Context& m_ctx ;
	QStringList m_downloadList ;
	QLineEdit& m_lineEdit ;
	QTableWidget& m_table ;
	QPushButton& m_cancelButton ;
} ;

#endif