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

#ifndef LOGGER_H
#define LOGGER_H

#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>
#include <QDebug>

class Logger
{
public:
	class Data
	{
	public:
		bool isEmpty()
		{
			return m_lines.empty() ;
		}
		bool isEmpty() const
		{
			return m_lines.empty() ;
		}
		bool isNotEmpty()
		{
			return !this->isEmpty() ;
		}
		bool isNotEmpty() const
		{
			return !this->isEmpty() ;
		}
		const QString& lastText()
		{
			return m_lines.rbegin()->text() ;
		}
		const QString& lastText() const
		{
			return m_lines.rbegin()->text() ;
		}
		void clear()
		{
			m_lines.clear() ;
		}
		QString toString()
		{
			if( this->isNotEmpty() ){

				auto it = m_lines.begin() ;

				auto m = it->text() ;

				it++ ;

				for( ; it != m_lines.end() ; it++ ){

					m += "\n" + it->text() ;
				}

				return m ;
			}else{
				return {} ;
			}
		}
		void removeLast()
		{
			m_lines.pop_back() ;
		}
		void replaceLast( const QString& e )
		{
			m_lines.rbegin()->replace( e ) ;
		}
		template< typename Function,
			  typename Engine >
		void replaceOrAdd( Function function,const Engine& engine,const QString& text,int id )
		{
			for( auto it = m_lines.rbegin() ; it != m_lines.rend() ; it++ ){

				if( function( engine,it->text() ) && it->id() == id ){

					it->replace( text ) ;

					return ;
				}
			}

			this->add( text,id ) ;
		}
		template< typename Text >
		void add( Text&& text,int s = -1 )
		{
			m_lines.emplace_back( std::forward< Text >( text ),s ) ;
		}
	private:
		class line
		{
		public:
			line( const QString& text,int id ) :
				m_text( text ),
				m_id( id )
			{
			}
			line( const QString& text ) :
				m_text( text ),
				m_id( -1 )
			{
			}
			const QString& text() const
			{
				return m_text ;
			}
			const QString& text()
			{
				return m_text ;
			}
			int id()
			{
				return m_id ;
			}
			int id() const
			{
				return m_id ;
			}
			void replace( const QString& text )
			{
				m_text = text ;
			}
		private:
			QString m_text ;
			int m_id ;
		} ;
		std::vector< Logger::Data::line > m_lines ;
	} ;

	Logger( QPlainTextEdit& ) ;
	void add( const QString&,int id = -1 ) ;
	void clear() ;
	template< typename Function >
	void add( const Function& function,int id )
	{
		function( m_lines,id ) ;
		this->update() ;
	}
	Logger( const Logger& ) = delete ;
	Logger& operator=( const Logger& ) = delete ;
	Logger( Logger&& ) = delete ;
	Logger& operator=( Logger&& ) = delete ;
private:
	void update() ;
	QPlainTextEdit& m_textEdit ;
	Logger::Data m_lines ;
} ;

class LoggerWrapper
{
public:
	LoggerWrapper() = delete ;
	LoggerWrapper( Logger& logger,int id ) :
		m_logger( &logger ),
		m_id( id )
	{
	}
	void add( const QString& e )
	{
		m_logger->add( e,m_id ) ;
	}
	void clear()
	{
		m_logger->clear() ;
	}
	template< typename Function >
	void add( const Function& function )
	{
		m_logger->add( function,m_id ) ;
	}
private:
	Logger * m_logger ;
	int m_id ;
};

template< typename Function,
	  typename Engine >
class loggerBatchDownloader
{
public:
	loggerBatchDownloader( Function function,
			       Engine& engine,
			       Logger& logger,
			       QTableWidgetItem& item,
			       int id ) :
		m_tableWidgetItem( item ),
		m_function( std::move( function ) ),
		m_engine( engine ),
		m_logger( logger ),
		m_id( id )
	{
	}
	void add( const QString& s )
	{
		m_logger.add( s,m_id ) ;

		if( s.startsWith( "[media-downloader]" ) ){

			m_lines.add( s ) ;
		}else{
			m_lines.add( "[media-downloader] " + s ) ;
		}

		this->update() ;
	}
	void clear()
	{
		m_tableWidgetItem.setText( "" ) ;
		m_lines.clear() ;
	}
	template< typename F >
	void add( const F& function )
	{
		m_logger.add( function,m_id ) ;
		function( m_lines,-1 ) ;
		this->update() ;
	}
private:
	void update()
	{
		if( m_lines.isNotEmpty() ){

			auto& function = *m_function ;
			m_tableWidgetItem.setText( function( m_engine,m_lines.lastText() ) ) ;
		}
	}
	QTableWidgetItem& m_tableWidgetItem ;
	Function m_function ;
	Engine& m_engine ;
	Logger& m_logger ;
	Logger::Data m_lines ;
	int m_id ;
} ;

template< typename Function,typename Engine >
static auto make_loggerBatchDownloader( Function function,
					Engine& engine,
					Logger& logger,
					QTableWidgetItem& item,
					int id )
{
	return loggerBatchDownloader< Function,Engine >( std::move( function ),engine,logger,item,id ) ;
}

class loggerPlaylistDownloader
{
public:
	loggerPlaylistDownloader( QTableWidget& t,
				  const QFont& f,
				  Logger& logger,
				  const QString& u,
				  int id ) :
		m_table( t ),
		m_font( f ),
		m_logger( logger ),
		m_urlPrefix( u ),
		m_id( id )
	{
		this->clear() ;
	}
	void add( const QString& e )
	{
		m_logger.add( e,m_id ) ;
	}
	void clear()
	{
		auto s = m_table.rowCount() ;

		for( int i = 0 ; i < s ; i++ ){

			m_table.removeRow( 0 ) ;
		}

		m_lines.clear() ;
	}
	template< typename Function >
	void add( const Function& function )
	{
		m_logger.add( function,m_id ) ;
		function( m_lines,-1 ) ;
		this->update() ;
	}
private:
	void update()
	{
		if( m_lines.isNotEmpty() ){

			auto row = m_table.rowCount() ;

			m_table.insertRow( row ) ;

			auto item = new QTableWidgetItem() ;

			item->setText( m_urlPrefix + m_lines.lastText() ) ;

			item->setTextAlignment( Qt::AlignCenter ) ;
			item->setFont( m_font ) ;

			m_table.setItem( row,0,item ) ;
		}
	}
private:
	QTableWidget& m_table ;
	const QFont& m_font ;
	Logger& m_logger ;
	const QString& m_urlPrefix ;
	Logger::Data m_lines ;
	int m_id ;
};

#endif
