#include "Map/TLMapBlock.h"
#include "MC/AssetsManager.h"
#include "MC/MCLoader.h"
#include "map.pb.h"

#define GRID_SPRITE_FILE "images/GridLine.png"

TLMapBlock::TLMapBlock( const std::string& strMapBlockFile )
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    m_strMapBlockFile = strMapBlockFile;
    m_nRow = 0;
    m_nCol = 0;
    m_bShowGridLine = false;

	CCTexture2D* pGridTexture = MCLoader::sharedMCLoader()->loadTexture( GRID_SPRITE_FILE );
    m_pGridBatchNode = CCSpriteBatchNode::createWithTexture( pGridTexture );
    addChild( m_pGridBatchNode, 1000 );
#endif
}

TLMapBlock::~TLMapBlock()
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    clear();
#endif
}

TLMapBlock* TLMapBlock::create( const std::string& strFileName )
{
    TLMapBlock * pRet = new TLMapBlock( strFileName );
    if( pRet && pRet->init() )
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}

bool TLMapBlock::newMapBlock( const std::string& strMapBlockFile, int nRow, int nCol, int nGridWidth, int nGridHeight )
{
    FILE* fp = fopen( strMapBlockFile.c_str(), "wb" );
    if( fp == NULL )
        return false;

    framework::MapBlock mbData;
    mbData.set_row( nRow );
    mbData.set_col( nCol );
    mbData.set_width( nGridWidth );
    mbData.set_height( nGridHeight );

    //std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    //std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    //for( ; iter != iter_end; ++iter )
    //{
    //    SpriteInfo* pSpriteInfo = (*iter);

    //    framework::SpriteInfo* si = mbData.add_sprites();
    //    si.set_file( pSpriteInfo->strFileName );
    //    si.set_x( pSpriteInfo->x );
    //    si.set_y( pSpriteInfo->y );
    //    si.set_scale_x( pSpriteInfo->scale_x );
    //    si.set_scale_y( pSpriteInfo->scale_y );
    //    si.set_rotation( pSpriteInfo->rotation );
    //}
    
    for( int i=0; i < nRow * nCol; ++i )
        mbData.add_gridstates( 0 );

    std::string strBuffer;
    mbData.SerializeToString( &strBuffer );

    fwrite( strBuffer.c_str(), strBuffer.length(), 1, fp );

    fclose( fp );

    return true;
}

bool TLMapBlock::init()
{
    if( !CCNode::init() )
        return false;

	unsigned long iSize = 0;
	unsigned char* pBuffer = AssetsManager::sharedAssetsManager()->getFileContent( m_strMapBlockFile.c_str(), "rb", &iSize );
    if( pBuffer == NULL )
        return false;

    framework::MapBlock mbData;
	mbData.ParseFromArray( (void*)pBuffer, iSize );

	delete[] pBuffer;

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    // 先清理
    clear();

    // 
    m_nRow = mbData.row();
    m_nCol = mbData.col();
    m_nWidth = mbData.width();
    m_nHeight = mbData.height();
#endif

    for( int i=0; i < mbData.sprites_size(); ++i )
    {
        const framework::SpriteInfo& si = mbData.sprites( i );

        CCSprite* pSprite = MCLoader::sharedMCLoader()->loadSprite( si.file().c_str() );
        pSprite->setPositionX( si.x() );
        pSprite->setPositionY( si.y() );
        pSprite->setScaleX( si.scale_x() );
        pSprite->setScaleY( si.scale_y() );
        pSprite->setRotation( si.rotation() );
        addChild( pSprite );

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
        SpriteInfo* pSpriteInfo = new SpriteInfo;
        pSpriteInfo->pSprite = pSprite;
        pSpriteInfo->strFileName = si.file();
        pSpriteInfo->x = si.x();
        pSpriteInfo->y = si.y();
        pSpriteInfo->scale_x = si.scale_x();
        pSpriteInfo->scale_y = si.scale_y();
        pSpriteInfo->rotation = si.rotation();

        m_listAllSprites.push_back( pSpriteInfo );
#endif
    }

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    m_vecGridStates.resize( m_nRow * m_nCol, 0 );
#endif

    return true;
}

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
void TLMapBlock::save()
{
    FILE* fp = fopen( m_strMapBlockFile.c_str(), "wb" );
    if( fp != NULL )
    {
        framework::MapBlock mbData;
        mbData.set_row( m_nRow );
        mbData.set_col( m_nCol );
        mbData.set_width( m_nWidth );
        mbData.set_height( m_nHeight );

        std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
        std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
        for( ; iter != iter_end; ++iter )
        {
            SpriteInfo* pSpriteInfo = (*iter);

            framework::SpriteInfo* si = mbData.add_sprites();
            si->set_file( pSpriteInfo->strFileName );
            si->set_x( pSpriteInfo->x );
            si->set_y( pSpriteInfo->y );
            si->set_scale_x( pSpriteInfo->scale_x );
            si->set_scale_y( pSpriteInfo->scale_y );
            si->set_rotation( pSpriteInfo->rotation );
        }

        for( int i=0; i < m_vecGridStates.size(); ++i )
            mbData.add_gridstates( m_vecGridStates[i] );

        std::string strBuffer;
        mbData.SerializeToString( &strBuffer );

        fwrite( strBuffer.c_str(), strBuffer.length(), 1, fp );

        fclose( fp );
    }
}

CCSprite* TLMapBlock::addSprite( const std::string& strFileName, float x, float y )
{
    CCSprite* pRetSprite = MCLoader::sharedMCLoader()->loadSprite( strFileName.c_str() );
	if( pRetSprite != NULL )
	{
		SpriteInfo* si = new SpriteInfo;
		si->pSprite = pRetSprite;
		si->strFileName = strFileName;
		si->x = x;
		si->y = y;
		si->scale_x = 0.0f;
		si->scale_y = 0.0f;
		si->rotation = 0.0f;

		m_listAllSprites.push_back( si );

		pRetSprite->setPosition( CCPoint( x, y ) );
		addChild( pRetSprite );
	}

	return pRetSprite;
}

void TLMapBlock::create( int nRow, int nCol, int nWidth, int nHeight )
{
    // 先清理
    clear();

    // 
    m_nRow = nRow;
    m_nCol = nCol;
    m_nWidth = nWidth;
    m_nHeight = nHeight;

    m_vecGridStates.resize( nRow * nCol, 0 );

    // 重新生成网格
    recreateGridLine();
}

void TLMapBlock::recreateGridLine()
{
    float fHalfTotalWidth = m_nCol * m_nWidth * 0.5f;
    float fHalfTotalHeight = m_nRow * m_nHeight * 0.5f;

    m_pGridBatchNode->removeAllChildrenWithCleanup( true );

    for( int i=0; i < m_nRow; ++i )
    {
        for( int j=0; j < m_nCol; ++j )
        {
            CCSprite* pSprite = MCLoader::sharedMCLoader()->loadSprite( GRID_SPRITE_FILE );
            //const CCSize& size = pSprite->getContentSize();

            float x = ( ( j + 0.5f ) * m_nWidth ) - fHalfTotalWidth;
            float y = fHalfTotalHeight - ( ( i + 0.5f ) * m_nHeight );
            pSprite->setPosition( CCPoint( x, y ) );

            m_pGridBatchNode->addChild( pSprite );
        }
    }
}

void TLMapBlock::setIsShowGirdLine( bool bIsShow )
{
    m_bShowGridLine = bIsShow;
    m_pGridBatchNode->setVisible( bIsShow );
}

void TLMapBlock::clear()
{
    m_nRow = 0;
    m_nCol = 0;

    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        delete pSpriteInfo;
    }
    m_listAllSprites.clear();

    m_vecGridStates.clear();
}
#endif
