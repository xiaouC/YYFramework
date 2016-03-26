#pragma once
#include <sprite_nodes/CCSprite.h>
#include <vector>

// grid flag ------------------------------------------------------------------------------------
enum TL_GRID_FLAG
{
	TL_GRID_FLAG_PASS				    = 0x00000001,					// 能否通行
};

USING_NS_CC;

class TLMapBlock : public CCNode
{
public:
	TLMapBlock( const std::string& strMapBlockFile );
	virtual ~TLMapBlock();

    static TLMapBlock* create( const std::string& strFileName );

    virtual bool init();

public:
    bool load( const std::string& strFileName );

protected:
    CCSpriteBatchNode* m_pMaterialBatchNode;

    void updateMaterial( const std::string& strMaterial, int nBlockWidth, int nBlockHeight );

    // 下面的是编辑器独有的
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX )
public:
    static bool newMapBlock( const std::string& strMapBlockFile, int nRow, int nCol, int nGridWidth, int nGridHeight, const std::string& strMaterial );

    void create( int nRow, int nCol, int nWidth, int nHeight );
    void clear();
    void save();

    void setMaterial( const std::string& strMaterial );

	CCSprite* addSprite( const std::string& strFileName, float x, float y );
	void removeSprite( CCSprite* pSprite );
	void moveSprite( CCSprite* pSprite, float mv_x, float mv_y );
	void scaleSprite( CCSprite* pSprite, float scale );
	void rotateSprite( CCSprite* pSprite, float rotation );

    // 
    CCSprite* hitSprite( float x, float y );

    void recreateGridLine();
    void setIsShowGirdLine( bool bIsShow );
    bool getIsShowGridLine() const { return m_bShowGridLine; }

protected:
    std::string m_strMapBlockFile;
    int m_nRow, m_nCol;
    int m_nWidth, m_nHeight;
    std::string m_strMaterial;

    struct SpriteInfo {
        CCSprite*       pSprite;
        std::string     strFileName;
        float           x, y;
        float           scale;
        float           rotation;
        int             z_order;
    };
    std::list<SpriteInfo*> m_listAllSprites;
    std::vector<int> m_vecGridStates;

    bool m_bShowGridLine;
    CCSpriteBatchNode* m_pGridBatchNode;
#endif
};
