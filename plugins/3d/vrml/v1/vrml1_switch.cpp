/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Cirilo Bernardo <cirilo.bernardo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <iostream>
#include <iterator>

#include "vrml1_base.h"
#include "vrml1_switch.h"
#include "plugins/3dapi/ifsg_all.h"


WRL1SWITCH::WRL1SWITCH( NAMEREGISTER* aDictionary ) : WRL1NODE( aDictionary )
{
    m_Type = WRL1_SWITCH;
    whichChild = -1;

    return;
}


WRL1SWITCH::WRL1SWITCH( NAMEREGISTER* aDictionary, WRL1NODE* aParent ) :
    WRL1NODE( aDictionary )
{
    m_Type = WRL1_SWITCH;
    m_Parent = aParent;
    whichChild = -1;

    if( NULL != m_Parent )
        m_Parent->AddChildNode( this );

    return;
}


WRL1SWITCH::~WRL1SWITCH()
{
    #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 2 )
    std::cerr << " * [INFO] Destroying Switch with " << m_Children.size();
    std::cerr << " children, " << m_Refs.size() << " references and ";
    std::cerr << m_BackPointers.size() << " backpointers\n";
    #endif
    return;
}


// functions inherited from WRL1NODE
bool WRL1SWITCH::Read( WRLPROC& proc, WRL1BASE* aTopNode )
{
    /*
     * Structure of a Switch node:
     *
     * Switch {
     *      exposedField    SFInt32     whichChild     -1
     *      children
     * }
     */

    if( NULL == aTopNode )
    {
        #ifdef DEBUG_VRML1
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] aTopNode is NULL\n";
        #endif
        return false;
    }

    size_t line, column;
    proc.GetFilePosData( line, column );

    char tok = proc.Peek();

    if( proc.eof() )
    {
        #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; unexpected eof at line ";
        std::cerr << line << ", column " << column << "\n";
        #endif
        return false;
    }

    if( '{' != tok )
    {
        #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
        std::cerr << proc.GetError() << "\n";
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; expecting '{' but got '" << tok;
        std::cerr  << "' at line " << line << ", column " << column << "\n";
        #endif

        return false;
    }

    proc.Pop();
    std::string glob;

    while( true )
    {
        char pchar = proc.Peek();

        if( pchar == '}' )
        {
            proc.Pop();
            break;
        }
        else if ( pchar == 'w' )
        {
            if( !proc.ReadName( glob ) )
            {
                #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
                std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                std::cerr << proc.GetError() <<  "\n";
                #endif

                return false;
            }

            if( !glob.compare( "whichChild" ) )
            {
                if( !proc.ReadSFInt( whichChild ) )
                {
                    #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
                    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                    std::cerr << " * [INFO] invalid whichChild at line " << line << ", column ";
                    std::cerr << column << "\n";
                    std::cerr << " * [INFO] file: '" << proc.GetFileName() << "'\n";
                    std::cerr << " * [INFO] message: '" << proc.GetError() << "'\n";
                    #endif
                    return false;
                }

                continue;
            }

            #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] invalid Switch at line " << line << ", column ";
            std::cerr << column << " (expected 'whichChild')\n";
            std::cerr << " * [INFO] file: '" << proc.GetFileName() << "'\n";
            #endif

            return false;
        }

        proc.GetFilePosData( line, column );

        if( !aTopNode->ReadNode( proc, this, NULL ) )
        {
            #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad file format; unexpected eof at line ";
            std::cerr << line << ", column " << column << "\n";
            #endif

            return false;
        }

        if( proc.Peek() == ',' )
            proc.Pop();

    }   // while( true ) -- reading contents of Switch{}

    return true;
}


SGNODE* WRL1SWITCH::TranslateToSG( SGNODE* aParent, bool calcNormals )
{
    #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 2 )
    std::cerr << " * [INFO] Translating Switch with " << m_Children.size();
    std::cerr << " children, " << m_Refs.size() << " references and ";
    std::cerr << m_BackPointers.size() << " backpointers (total ";
    std::cerr << m_Items.size() << " items)\n";
    #endif

    if( m_Items.empty() )
        return NULL;

    if( whichChild < 0 || whichChild >= (int)m_Items.size() )
        return NULL;

    std::list< WRL1NODE* >::iterator ip = m_Items.begin();
    std::advance( ip, whichChild );

    IFSG_TRANSFORM txNode( aParent );

    if( WRL1_BASE != m_Parent->GetNodeType() )
        m_current = *( m_Parent->GetCurrentSettings() );
    else
        m_current.Init();

    return (*ip)->TranslateToSG( aParent, calcNormals );
}
