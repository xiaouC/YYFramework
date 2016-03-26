#include "Map/TLMapBlock.h"
#include "MC/AssetsManager.h"
#include "MC/MCLoader.h"
#include "map.pb.h"

#define GRID_SPRITE_FILE "images/GridLine.png"

TLMapBlock::TLMapBlock( const std::string& strMapBlockFile )
{
    m_pMaterialBatchNode = NULL;

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

bool TLMapBlock::newMapBlock( const std::string& strMapBlockFile, int nRow, int nCol, int nGridWidth, int nGridHeight, const std::string& strMaterial )
{
    FILE* fp = fopen( strMapBlockFile.c_str(), "wb" );
    if( fp == NULL )
        return false;

    framework::MapBlock mbData;
    mbData.set_row( nRow );
    mbData.set_col( nCol );
    mbData.set_width( nGridWidth );
    mbData.set_height( nGridHeight );
    mbData.set_material( strMaterial );

    //std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    //std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    //for( ; iter != iter_end; ++iter )
    //{
    //    SpriteInfo* pSpriteInfo = (*iter);

    //    framework::SpriteInfo* si = mbData.add_sprites();
    //    si.set_file( pSpriteInfo->strFileName );
    //    si.set_x( pSpriteInfo->x );
    //    si.set_y( pSpriteInfo->y );
    //    si.set_scale( pSpriteInfo->scale );
    //    si.set_rotation( pSpriteInfo->rotation );
    //    si.set_z_order( pSpriteInfo->z_order );
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
    m_strMaterial = mbData.material();
#endif

    // 地表材质
    std::string strMaterial = mbData.material();
    updateMaterial( strMaterial, mbData.col() * mbData.width(), mbData.row() * mbData.height() );

    // 地表装饰物
    for( int i=0; i < mbData.sprites_size(); ++i )
    {
        const framework::SpriteInfo& si = mbData.sprites( i );

        CCSprite* pSprite = MCLoader::sharedMCLoader()->loadSprite( si.file().c_str() );
        pSprite->setPositionX( si.x() );
        pSprite->setPositionY( si.y() );
        pSprite->setScale( si.scale() );
        pSprite->setRotation( si.rotation() );
        addChild( pSprite, si.z_order() );

#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
        SpriteInfo* pSpriteInfo = new SpriteInfo;
        pSpriteInfo->pSprite = pSprite;
        pSpriteInfo->strFileName = si.file();
        pSpriteInfo->x = si.x();
        pSpriteInfo->y = si.y();
        pSpriteInfo->scale = si.scale();
        pSpriteInfo->rotation = si.rotation();
        pSpriteInfo->z_order = si.z_order();

        m_listAllSprites.push_back( pSpriteInfo );
#endif
    }

    // 地表上摆放的物件

    // 地表网格的状态
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
    m_vecGridStates.resize( m_nRow * m_nCol, 0 );
#endif

    return true;
}

void TLMapBlock::updateMaterial( const std::string& strMaterial, int nBlockWidth, int nBlockHeight )
{
    if( m_pMaterialBatchNode != NULL )
    {
        m_pMaterialBatchNode->removeFromParentAndCleanup( true );
        m_pMaterialBatchNode = NULL;
    }

    // 地表材质
	CCTexture2D* pMaterialTexture = MCLoader::sharedMCLoader()->loadTexture( strMaterial.c_str() );
    if( pMaterialTexture == NULL )
        return;

    m_pMaterialBatchNode = CCSpriteBatchNode::createWithTexture( pMaterialTexture );
    addChild( m_pMaterialBatchNode, -1000 );

    CCSprite* pFirstMaterialSprite = MCLoader::sharedMCLoader()->loadSprite( strMaterial.c_str() );
    m_pMaterialBatchNode->addChild( pFirstMaterialSprite );
    const CCSize& size = pFirstMaterialSprite->getContentSize();

    int nRowCount = nBlockHeight / size.height;
    if( nRowCount * size.height < nBlockHeight )
        nRowCount = nRowCount + 1;

    int nColCount = nBlockWidth / size.width;
    if( nColCount * size.width < nBlockWidth )
        nColCount = nColCount + 1;

    int nRealWidth = nColCount * size.width;
    int nRealHeight = nRowCount * size.height;

    float x = ( size.width - nRealWidth ) * 0.5f;
    float y = ( nRealHeight - size.height ) * 0.5f;
    pFirstMaterialSprite->setPosition( CCPoint( x, y ) );

    for( int i=0; i < nRowCount; ++i )
    {
        float y = ( nRealHeight - size.height ) * 0.5f - i * size.height;
        for( int j=0; j < nColCount; ++j )
        {
            if( i==0 && j== 0 )
                continue;

            CCSprite* pMaterialSprite = MCLoader::sharedMCLoader()->loadSprite( strMaterial.c_str() );
            m_pMaterialBatchNode->addChild( pMaterialSprite );

            float x = ( size.width - nRealWidth ) * 0.5f + j * size.width;
            pMaterialSprite->setPosition( CCPoint( x, y ) );
        }
    }
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
            si->set_scale( pSpriteInfo->scale );
            si->set_rotation( pSpriteInfo->rotation );
            si->set_z_order( pSpriteInfo->z_order );
        }

        for( int i=0; i < (int)m_vecGridStates.size(); ++i )
            mbData.add_gridstates( m_vecGridStates[i] );

        std::string strBuffer;
        mbData.SerializeToString( &strBuffer );

        fwrite( strBuffer.c_str(), strBuffer.length(), 1, fp );

        fclose( fp );
    }
}

void TLMapBlock::setMaterial( const std::string& strMaterial )
{
    updateMaterial( strMaterial, m_nCol * m_nWidth, m_nRow * m_nHeight );
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
		si->scale = 0.0f;
		si->rotation = 0.0f;
        si->z_order = 0;

		m_listAllSprites.push_back( si );

		pRetSprite->setPosition( CCPoint( x, y ) );
		addChild( pRetSprite );
	}

	return pRetSprite;
}

void TLMapBlock::removeSprite( CCSprite* pSprite )
{
    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        if( pSpriteInfo->pSprite == pSprite )
        {
            pSprite->removeFromParentAndCleanup( true );

            m_listAllSprites.erase( iter );

            return;
        }
    }
}

void TLMapBlock::moveSprite( CCSprite* pSprite, float mv_x, float mv_y )
{
    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        if( pSpriteInfo->pSprite == pSprite )
        {
            pSpriteInfo->x = pSpriteInfo->x + mv_x;
            pSpriteInfo->y = pSpriteInfo->y + mv_y;
            pSpriteInfo->pSprite->setPosition( CCPoint( pSpriteInfo->x, pSpriteInfo->y ) );

            return;
        }
    }
}

void TLMapBlock::scaleSprite( CCSprite* pSprite, float scale )
{
    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        if( pSpriteInfo->pSprite == pSprite )
        {
            pSpriteInfo->scale = scale;
            pSpriteInfo->pSprite->setScale( pSpriteInfo->scale );

            return;
        }
    }
}

void TLMapBlock::rotateSprite( CCSprite* pSprite, float rotation )
{
    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        if( pSpriteInfo->pSprite == pSprite )
        {
            pSpriteInfo->rotation = rotation;
            pSpriteInfo->pSprite->setRotation( pSpriteInfo->rotation );

            return;
        }
    }
}

CCSprite* TLMapBlock::hitSprite( float x, float y )
{
    int nZOrder = -9999999;
    CCSprite* pRetSprite = NULL;

    std::list<SpriteInfo*>::iterator iter = m_listAllSprites.begin();
    std::list<SpriteInfo*>::iterator iter_end = m_listAllSprites.end();
    for( ; iter != iter_end; ++iter )
    {
        SpriteInfo* pSpriteInfo = (*iter);
        const CCPoint& position = pSpriteInfo->pSprite->getPosition();
        const CCSize& size = pSpriteInfo->pSprite->getContentSize();
        if( x >= position.x - size.width * 0.5f &&
            x <= position.x + size.width * 0.5f &&
            y >= position.y - size.height * 0.5f &&
            y <= position.y + size.height * 0.5f )
        {
            int z_order = pSpriteInfo->pSprite->getZOrder();
            if( z_order > nZOrder )
            {
                nZOrder = z_order;
                pRetSprite = pSpriteInfo->pSprite;
            }
        }
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
